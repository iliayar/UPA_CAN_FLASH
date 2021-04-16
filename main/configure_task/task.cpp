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

#include "config.h"
#include "fields.h"
#include "service_all.h"

ConfigurationTask::ConfigurationTask(std::shared_ptr<QLogger> logger)
    : QTask(logger) {
    QWidget* window = new QWidget();
    QHBoxLayout* main_layout = new QHBoxLayout(window);

    QTabWidget* tabs = new QTabWidget(window);
    main_layout->addWidget(tabs);
    for (auto& [name, fields] : m_config.fields) {
        QScrollArea* scroll = new QScrollArea(tabs);
        QGroupBox* group = new QGroupBox(tr("&Parameteres"), scroll);
        scroll->setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
        scroll->setWidget(group);
        QVBoxLayout* layout = new QVBoxLayout(group);
        QPushButton* read_all_btn = new QPushButton(tr("&Read all fields"));
        layout->addWidget(read_all_btn);
        group->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        scroll->hide();
        for (Field* field : fields) {
            field->init(this);
            field->setParent(group);
            field->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            connect(read_all_btn, &QPushButton::released, field, &Field::read);
            layout->addWidget(field);
        }
        tabs->addTab(scroll, QString::fromStdString(name));
        group->adjustSize();
    }

    QFrame* err_tab = new QFrame(tabs);
    QVBoxLayout* err_layouts = new QVBoxLayout(err_tab);
    QTextEdit* err_log = new QTextEdit(err_tab);
    QFrame* btns_frame = new QFrame(err_tab);
    QHBoxLayout* btns_layout = new QHBoxLayout(btns_frame);
    QPushButton* err1_btn = new QPushButton(tr("&Read TestFail (0x01)"));
    QPushButton* err2_btn = new QPushButton(tr("&Read Confirmed (0x08)"));
    QPushButton* err_clear_btn = new QPushButton(tr("&Clear errors"));

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

    tabs->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    tabs->adjustSize();
    window->setAttribute(Qt::WA_DeleteOnClose);
    window->adjustSize();
    window->resize(QGuiApplication::primaryScreen()->size());
    window->show();
    window->setFocus();
    qDebug() << tabs->size();
    m_window = window;
    m_err_log = err_log;
}

void ConfigurationTask::task() {

    auto response = call(Can::ServiceRequest::DiagnosticSessionControl::build()
                        ->subfunction(Can::ServiceRequest::DiagnosticSessionControl::
                                          Subfunction::extendDiagnosticSession)
                        ->build()
                        .value());

    IF_NEGATIVE(response) {
        LOG(error, "Failed ot enter extendDiagnosticSession");
        return;
    }
    QEventLoop loop;
    connect(m_window, &QWidget::destroyed, &loop, &QEventLoop::quit);
    loop.exec();
}

void ConfigurationTask::clear_errors() {
    auto response = call(Can::ServiceRequest::ClearDiagnosticInformation::build()
                            ->group(0xFFFFFF)
                            ->build()
                            .value());
    IF_NEGATIVE(response) {
        m_logger->error("Failed to clear errors");
    }
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
