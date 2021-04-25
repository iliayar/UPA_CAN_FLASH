#include "configuration_window.h"

#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QFileDialog>
#include <QGroupBox>
#include <QLabel>
#include <QListWidget>
#include <QMainWindow>
#include <QMenuBar>
#include <QPushButton>
#include <QScreen>
#include <QScrollArea>
#include <QTabWidget>
#include <QVBoxLayout>
#include <QMenuBar>

#include "fields.h"

ConfigurationWindow::ConfigurationWindow(QWidget* parent,
                                         ConfigurationTask* task)
    : m_task(task),
      QDialog(parent),
      m_settings("canFlash", "Some cool organization name") {
    QHBoxLayout* layout = new QHBoxLayout(this);
    QTabWidget* tabs = new QTabWidget();

    create_layout(tabs);

    layout->addWidget(tabs);
    QMenuBar* menu_bar = new QMenuBar(this);
    QMenu* tools_menu = new QMenu(tr("Tools"));
    QAction* reset_action = new QAction(tr("Factory reset"), this);

    tools_menu->addAction(reset_action);
    menu_bar->addMenu(tools_menu);

    layout->setMenuBar(menu_bar);
    connect(reset_action, &QAction::triggered, this,
            &ConfigurationWindow::factory_reset);

    // setCentralWidget(tabs);
}

ConfigurationWindow::~ConfigurationWindow() {}

void ConfigurationWindow::create_layout(QTabWidget* tabs) {
    for (auto& [name, fields] : m_config.fields) {
        QScrollArea* scroll = new QScrollArea(tabs);
        QGroupBox* group = new QGroupBox(tr("&Parameteres"), scroll);
        scroll->setHorizontalScrollBarPolicy(
            Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
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
            field->setParent(group);
            field->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Maximum);
            connect(read_all_btn, &QPushButton::released, field, &Field::read);
            connect(write_all_btn, &QPushButton::released, field,
                    &Field::write);
            layout->addWidget(field);
        }
        std::string groupname = name;
        // connect(load_btn, &QPushButton::released,
        //         [this, groupname]() {
        //             QString path =
        //             m_settings.value("configuration/data").toString();
        //             QString filename = QFileDialog::getOpenFileName(
        //                 this->m_window, tr("Open configuration data"), path);
        //             this->m_config.json_to_group(filename, groupname);
        //             if(filename != "") {
        //                 m_settings.setValue("configuration/data", filename);
        //             }
        //         });
        // connect(dump_btn, &QPushButton::released,
        //         [this, groupname]() {
        //             QString path =
        //             m_settings.value("configuration/data").toString();
        //             QString filename = QFileDialog::getSaveFileName(
        //                 this->m_window, tr("Save configuration data"), path);
        //             this->m_config.group_to_json(filename, groupname);
        //             if(filename != "") {
        //                 m_settings.setValue("configuration/data", filename);
        //             }
        //         });
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

    connect(err1_btn, &QPushButton::released,
            [this]() { this->read_errors(Can::testFailedDTC); });
    connect(err2_btn, &QPushButton::released,
            [this]() { this->read_errors(Can::confirmedDTC); });
    connect(err_clear_btn, &QPushButton::released, this,
            &ConfigurationWindow::clear_errors);

    tabs->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    tabs->adjustSize();
    adjustSize();
    // resize(QGuiApplication::primaryScreen()->size());
    m_err_log = err_log;
}

void ConfigurationWindow::clear_errors() {
    m_err_log->clear();
    if(m_task->clear_errors()) {
        m_err_log->append("OK");
    } else {
        m_err_log->append("FAIL");
    }
}

void ConfigurationWindow::factory_reset() {
    m_task->factory_reset();
}

void ConfigurationWindow::read_errors(uint8_t dtc) {
    auto errors = m_task->read_errors(dtc);
    m_err_log->clear();
    if(!errors) {
        m_err_log->append("Failed to read errors");
    }
    for (auto err : errors.value()) {
        auto it1 = m_config.errors.find(err->get_type());
        if (it1 == m_config.errors.end()) {
            continue;
        }
        auto& [name, var_map] = it1->second;
        auto it2 = var_map.find(err->get_status());
        if (it2 == var_map.end()) {
            continue;
        }
        auto& [mnemonic, description] = it2->second;
        m_err_log->append(
            QString::fromStdString(name + " " + mnemonic + " " + description));
    }
}
