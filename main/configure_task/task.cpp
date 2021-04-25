#include "task.h"

#include <QDialog>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QTabWidget>
#include <QMainWindow>
#include <QApplication>
#include <QScreen>
#include <QMenuBar>
#include <QFileDialog>

#include "config.h"
#include "fields.h"
#include "service_all.h"
#include "security.h"

ConfigurationTask::ConfigurationTask(std::shared_ptr<QLogger> logger, bool security)
    : QTask(logger), m_security(security), m_settings("canFlash", "Some cool organization name") {
    ConfigurationWindow* window = new ConfigurationWindow();

    QTabWidget* tabs = new QTabWidget(window);
    window->setCentralWidget(tabs);
    for (auto& [name, fields] : m_config.fields) {
        QScrollArea* scroll = new QScrollArea(tabs);
        QGroupBox* group = new QGroupBox(tr("&Parameteres"), scroll);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        scroll->setWidget(group);
        QVBoxLayout* layout = new QVBoxLayout(group);
        QFrame* btns_frame = new QFrame(group);
        QHBoxLayout* btns_layout = new QHBoxLayout(btns_frame);
        QPushButton* read_all_btn = new QPushButton(tr("&Read all fields"));
        QPushButton* write_all_btn = new QPushButton(tr("&Write all fields"));
        QPushButton* load_btn = new QPushButton(tr("&Read from file"));
        QPushButton* dump_btn = new QPushButton(tr("&Write to file"));
        btns_layout->addWidget(read_all_btn);
        btns_layout->addWidget(write_all_btn);
        btns_layout->addWidget(load_btn);
        btns_layout->addWidget(dump_btn);
        layout->addWidget(btns_frame);
        group->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        scroll->hide();
        for (Field* field : fields) {
            field->init(this);
            // field->setParent(group);
            field->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            connect(read_all_btn, &QPushButton::released, field, &Field::read);
            connect(write_all_btn, &QPushButton::released, field, &Field::write);
            layout->addWidget(field);
        }
        std::string groupname = name;
        connect(load_btn, &QPushButton::released,
                [this, groupname]() {
                    QString path = m_settings.value("configuration/data").toString();
                    QString filename = QFileDialog::getOpenFileName(
                        this->m_window, tr("Open configuration data"), path);
                    this->m_config.json_to_group(filename, groupname);
                    if(filename != "") {
                        m_settings.setValue("configuration/data", filename);
                    }
                });
        connect(dump_btn, &QPushButton::released,
                [this, groupname]() {
                    QString path = m_settings.value("configuration/data").toString();
                    QString filename = QFileDialog::getSaveFileName(
                        this->m_window, tr("Save configuration data"), path);
                    this->m_config.group_to_json(filename, groupname);
                    if(filename != "") {
                        m_settings.setValue("configuration/data", filename);
                    }
                });
        tabs->addTab(scroll, QString::fromStdString(name));
        group->adjustSize();
    }

    QMenu* tools_menu = new QMenu(tr("Tools"));
    QAction* reset_action = new QAction(tr("Factory reset"), window);

    QFrame* err_tab = new QFrame(tabs);
    QVBoxLayout* err_layouts = new QVBoxLayout(err_tab);
    QTextEdit* err_log = new QTextEdit(err_tab);
    QFrame* btns_frame = new QFrame(err_tab);
    QHBoxLayout* btns_layout = new QHBoxLayout(btns_frame);
    QPushButton* err1_btn = new QPushButton(tr("&Read TestFail (0x01)"));
    QPushButton* err2_btn = new QPushButton(tr("&Read Confirmed (0x08)"));
    QPushButton* err_clear_btn = new QPushButton(tr("&Clear errors"));

    tools_menu->addAction(reset_action);
    window->menuBar()->addMenu(tools_menu);

    err_log->setReadOnly(true);
    
    btns_layout->addWidget(err1_btn);
    btns_layout->addWidget(err2_btn);
    btns_layout->addWidget(err_clear_btn);
    err_layouts->setAlignment(Qt::AlignTop);
    err_layouts->addWidget(btns_frame);
    err_layouts->addWidget(err_log);

    tabs->addTab(err_tab, "Errors (DTC Controls)");

    connect(err1_btn, &QPushButton::released, [this]() {
        this->read_errors(Can::testFailedDTC);
    });
    connect(err2_btn, &QPushButton::released, [this]() {
        this->read_errors(Can::confirmedDTC);
    });
    connect(err_clear_btn, &QPushButton::released, this, &ConfigurationTask::clear_errors);
    connect(reset_action, &QAction::triggered, this, &ConfigurationTask::factory_reset);

    tabs->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    tabs->adjustSize();
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->adjustSize();
    window->resize(QGuiApplication::primaryScreen()->size());
    m_window = window;
    m_err_log = err_log;
}

void ConfigurationTask::factory_reset() {
    auto response = call(
        Can::ServiceRequest::RoutineControl::build()
            ->subfunction(
                Can::ServiceRequest::RoutineControl::Subfunction::StartRoutine)
            ->routine(Can::Routine::build()
                          ->id(0x201f) // FIXME Factory reset
                          ->data({})
                          ->build()
                          .value())
            ->build()
            .value());
    IF_NEGATIVE(response) {
        m_logger->error("Failed to perform Factory reset");
    }
    m_logger->info("Factory reset done");
}

void ConfigurationTask::task() {

    if(m_security) {
        if(!security_access(Crypto::SecuritySettings::get_mask03())) {
            return;
        }
    }

    QEventLoop loop;
    connect(m_window, &ConfigurationWindow::closed, &loop, &QEventLoop::quit);
    m_window->show();
    m_window->setFocus();
    loop.exec();
}

void ConfigurationTask::clear_errors() {
    auto response = call(Can::ServiceRequest::ClearDiagnosticInformation::build()
                            ->group(0xFFFFFF)
                            ->build()
                            .value());
    m_err_log->clear();
    IF_NEGATIVE(response) {
        m_err_log->append("FAIL");
        m_logger->error("Failed to clear errors");
    }
    m_err_log->append("OK");
}

void ConfigurationTask::read_errors(uint8_t mask) {
    auto response =
        call(Can::ServiceRequest::ReadDTCInformation::build()
                 ->subfunction(Can::ServiceRequest::ReadDTCInformation::
                                   Subfunction::reportDTCByStatusMask)
                 ->mask(mask)
                 ->build()
                 .value());
    IF_NEGATIVE(response) {
        m_logger->error("Failed to read errors");
        return;
    }
    m_err_log->clear();
    for(auto err : std::static_pointer_cast<Can::ServiceResponse::ReadDTCInformation>(response)->get_records()) {
        auto it1 = m_config.errors.find(err->get_type());
        if(it1 == m_config.errors.end()) {
            continue;
        }
        auto& [name, var_map] = it1->second;
        auto it2 = var_map.find(err->get_status());
        if(it2 == var_map.end()) {
            continue;
        }
        auto& [mnemonic, description] = it2->second;
        m_err_log->append(QString::fromStdString(name + " " + mnemonic + " " + description));
    }
}
