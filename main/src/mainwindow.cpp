#include "mainwindow.h"
#include <qboxlayout.h>
#include <qcombobox.h>
#include <qframe.h>
#include <qlabel.h>
#include <qlineedit.h>
#include <qmainwindow.h>
#include <qobjectdefs.h>
#include <qpushbutton.h>
#include <qspinbox.h>
#include <qtextedit.h>
#include <qvariant.h>
#include <sys/types.h>

#include <QCanBus>
#include <QCanBusFrame>
#include <QComboBox>
#include <QContextMenuEvent>
#include <QCoreApplication>
#include <QDebug>
#include <QFileDialog>
#include <QFontDatabase>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QMenu>
#include <QMenuBar>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QSizePolicy>
#include <QSpinBox>
#include <QTextEdit>
#include <QThread>
#include <QVBoxLayout>
#include <QCheckBox>
#include <QShortcut>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include "communicator.h"
#include "flash.h"
#include "frame.h"
#include "hex.h"
#include "logger.h"
#include "qcommunicator.h"
#include "qtask.h"
#include "task.h"
#include "security.h"
#include "configure_task/task.h"
#include "util.h"
#include "configure_task/configuration_window.h"

#ifdef __MINGW32__
#define CAN_PLUGINS                                         \
    {                                                       \
        {"sysWORXX", "systeccan"}, {"IXXAT", "ixxatcan"}, { \
            "VECTOR", "vectorcan"                           \
        }, { "PassThru", "passthrucan" }                    \
    }
#elif __linux__
#define CAN_PLUGINS                  \
    {                                \
        { "SocketCAN", "socketcan" } \
    }
#endif

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_device(),
      m_settings("canFlash", "Some cool organization name"),
      m_checker(nullptr) {
    QWidget* window = new QWidget();

    create_layout(window);

    bool ok;
    uint32_t mask = m_settings.value("crypto/mask02").toString().toUInt(&ok, 0);
    if(ok) Crypto::SecuritySettings::set_mask02(mask);
    else {
        m_settings.setValue("crypto/mask02", QString::number(Crypto::SecuritySettings::get_mask02()));
    }
    mask = m_settings.value("crypto/mask03").toString().toUInt(&ok, 0);
    if(ok) Crypto::SecuritySettings::set_mask03(mask);
    else {
        m_settings.setValue("crypto/mask03", QString::number(Crypto::SecuritySettings::get_mask03()));
    }

    if(!m_settings.contains("settings/postfix")) {
        m_settings.setValue("settings/postfix", "default");
    }

    if(!m_settings.contains("settings/checker_id")) {
        m_settings.setValue("settings/checker_id", 0x5E9);
    }

    m_device = nullptr;

    m_communicator =
        new QCommunicator(std::make_shared<QLogger>(m_logger_worker));
    connect(m_communicator, &QCommunicator::fetch_frame, this,
            &MainWindow::check_frames_to_write);
    connect(this, &MainWindow::frame_received, m_communicator,
            &QCommunicator::push_frame);
    connect(m_communicator, &QCommunicator::task_exited, this,
            &MainWindow::task_done);
    connect(this, &MainWindow::set_task, m_communicator,
            &QCommunicator::set_task);
    m_communicator->moveToThread(&m_communicator_thread);
    m_communicator_thread.start();

    setCentralWidget(window);

    restoreGeometry(m_settings.value("mainWindow/geometry").toByteArray());
    restoreState(m_settings.value("mainWindow/windowState").toByteArray());
}

void MainWindow::closeEvent(QCloseEvent *event) {
    m_settings.setValue("mainWindow/geometry", saveGeometry());
    m_settings.setValue("mainWindow/windowState", saveState());
    m_settings.setValue("task/testerId", m_tester_id_box->value());
    m_settings.setValue("task/ecuId", m_ecu_id_box->value());
    m_settings.setValue("device/bitrate", m_bitrate_list->currentText());
    m_settings.setValue("settings/postfix", m_config_postfix->text());
    m_settings.setValue("settings/checker_id", (uint)m_config_checker_id->value());
    m_settings.setValue("settings/security", (int)m_config_security_checkbox->isChecked());

    m_communicator_thread.terminate();
    m_communicator_thread.wait();
    QMainWindow::closeEvent(event);
}

void MainWindow::create_layout(QWidget* root) {
    DEBUG(info, "Creating layout");
    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    // Creating GUI Windgets

    QMenu* file_menu = new QMenu(tr("File"));

    QGroupBox* log_group = new QGroupBox(tr("Logs"));
    QGroupBox* options_group = new QGroupBox(tr("Options"));
    QGroupBox* message_log_progress_group = new QGroupBox();
    QGroupBox* file_group = new QGroupBox(tr("File"));
    QGroupBox* devices_group = new QGroupBox(tr("Devices"));
    QFrame* devices_buttons_group = new QFrame();
    QFrame* devices_list_buttons_group = new QFrame();
    QGroupBox* tasks_group = new QGroupBox(tr("Tasks"));
    QFrame* tester_id_frame = new QFrame();
    QFrame* ecu_id_frame = new QFrame();

    QHBoxLayout* main_layout = new QHBoxLayout(root);
    QHBoxLayout* log_layout = new QHBoxLayout(log_group);
    QVBoxLayout* message_log_progress_layout =
        new QVBoxLayout(message_log_progress_group);
    QVBoxLayout* options_layout = new QVBoxLayout(options_group);
    QVBoxLayout* file_layout = new QVBoxLayout(file_group);
    QVBoxLayout* devices_group_layout = new QVBoxLayout(devices_group);
    QHBoxLayout* devices_buttons_layout =
        new QHBoxLayout(devices_buttons_group);
    QHBoxLayout* devices_list_buttons_layout = new QHBoxLayout(devices_list_buttons_group);
    QVBoxLayout* tasks_group_layout = new QVBoxLayout(tasks_group);
    QHBoxLayout* tester_id_layout = new QHBoxLayout(tester_id_frame);
    QHBoxLayout* ecu_id_layout = new QHBoxLayout(ecu_id_frame);

    QTextEdit* log_frames = new QTextEdit();
    QTextEdit* log_messages = new QTextEdit();

    QProgressBar* progress_bar = new QProgressBar();

    QComboBox* plugins_list = new QComboBox();
    QComboBox* devices_list = new QComboBox(devices_group);
    QComboBox* bitrate_list = new QComboBox(devices_group);
    std::vector<QPushButton*> tasks_btns;

    QPushButton* checker_btn = new QPushButton("Search");
    QPushButton* refresh_btn = new QPushButton("Refresh");
    QPushButton* device_connect_btn = new QPushButton("Connect");
    QPushButton* device_disconnect_btn = new QPushButton("Disconnect");

    QShortcut* connect_shortcut = new QShortcut(QKeySequence("F2"), this);
    QShortcut* disconnect_shortcut = new QShortcut(QKeySequence("Esc"), this);

    QSpinBox* tester_id_box = new QSpinBox();
    QSpinBox* ecu_id_box = new QSpinBox();

    QSizePolicy log_policy = log_group->sizePolicy();
    QSizePolicy options_policy = options_group->sizePolicy();

    QLabel* filename_label = new QLabel("Choose file..");
    QLabel* crc_label = new QLabel("CRC: ???");
    QLabel* size_label = new QLabel("Size: ???");
    QLabel* addr_label = new QLabel("Begin address: ???");

    QWidget* settings_window = new QWidget(nullptr);

    QFrame* settings_mask02_frame = new QFrame();
    QFrame* settings_mask03_frame = new QFrame();
    QFrame* config_security_frame = new QFrame();
    QFrame* config_postfix_frame = new QFrame();
    QFrame* config_chekcer_id_frame = new QFrame();

    QVBoxLayout* settings_window_layout = new QVBoxLayout(settings_window);
    QHBoxLayout* settings_mask02_layout = new QHBoxLayout(settings_mask02_frame);
    QHBoxLayout* settings_mask03_layout = new QHBoxLayout(settings_mask03_frame);
    QHBoxLayout* config_securiry_layout = new QHBoxLayout(config_security_frame);
    QHBoxLayout* config_postfix_layout = new QHBoxLayout(config_postfix_frame);
    QHBoxLayout* config_checker_id_layout = new QHBoxLayout(config_chekcer_id_frame);

    QLineEdit* mask02_box = new QLineEdit();
    QLineEdit* mask03_box = new QLineEdit();
    QCheckBox* config_security_checkbox = new QCheckBox(config_security_frame);
    QLineEdit* config_postfix = new QLineEdit();
    QSpinBox* config_checker_id = new QSpinBox();

    // Creating layout
    
    log_policy.setHorizontalStretch(6);
    options_policy.setHorizontalStretch(1);

    main_layout->addWidget(log_group);
    main_layout->addWidget(options_group);

    log_layout->addWidget(message_log_progress_group);
    log_layout->addWidget(log_frames);

    message_log_progress_layout->addWidget(log_messages);
    message_log_progress_layout->addWidget(progress_bar);

    options_layout->addWidget(plugins_list);
    options_layout->addWidget(file_group);

    file_layout->addWidget(filename_label);
    file_layout->addWidget(crc_label);
    file_layout->addWidget(size_label);
    file_layout->addWidget(addr_label);

    options_layout->addWidget(devices_group);
    options_layout->addWidget(tasks_group);

    devices_group_layout->addWidget(devices_list);
    devices_group_layout->addWidget(bitrate_list);
    devices_group_layout->addWidget(devices_list_buttons_group);
    devices_list_buttons_layout->addWidget(checker_btn);
    devices_list_buttons_layout->addWidget(refresh_btn);
    devices_group_layout->addWidget(devices_buttons_group);

    devices_buttons_layout->addWidget(device_connect_btn);
    devices_buttons_layout->addWidget(device_disconnect_btn);

    tester_id_layout->addWidget(new QLabel("Tester:"));
    tester_id_layout->addWidget(tester_id_box);

    ecu_id_layout->addWidget(new QLabel("ECU:"));
    ecu_id_layout->addWidget(ecu_id_box);

    tasks_group_layout->addWidget(tester_id_frame);
    tasks_group_layout->addWidget(ecu_id_frame);

    settings_window_layout->addWidget(settings_mask02_frame);
    settings_window_layout->addWidget(settings_mask03_frame);
    settings_window_layout->addWidget(config_security_frame);
    settings_window_layout->addWidget(config_postfix_frame);
    settings_window_layout->addWidget(config_chekcer_id_frame);

    settings_mask02_layout->addWidget(new QLabel("MASK02"));
    settings_mask02_layout->addWidget(mask02_box);
    settings_mask03_layout->addWidget(new QLabel("MASK03"));
    settings_mask03_layout->addWidget(mask03_box);
    config_securiry_layout->addWidget(new QLabel("Security"));
    config_securiry_layout->addWidget(config_security_checkbox);
    config_postfix_layout->addWidget(new QLabel("Config postfix"));
    config_postfix_layout->addWidget(config_postfix);
    config_checker_id_layout->addWidget(new QLabel("Search Message ID"));
    config_checker_id_layout->addWidget(config_checker_id);

    // Setting up widgets

    DEBUG(info, "Setting up widgets");

    for(QString task : {"Flash" /*, "Test"*/, "Configuration"}) {
        QPushButton* btn = new QPushButton(task, tasks_group);
        if(task != "Configuration") {
            btn->setDisabled(true);
        }
        tasks_btns.push_back(btn);
        tasks_group_layout->addWidget(btn);
    }

    log_group->setSizePolicy(log_policy);
    options_group->setSizePolicy(options_policy);

    m_file_menu_act = new QAction(tr("&Choose"), this);
    m_settings_menu_act = new QAction(tr("&Settings"), this);

    file_menu->addAction(m_file_menu_act);
    file_menu->addAction(m_settings_menu_act);
    menuBar()->addMenu(file_menu);

    log_frames->setReadOnly(true);
    log_frames->setFont(fixedFont);
    log_messages->setReadOnly(true);
    log_messages->setFont(fixedFont);
    progress_bar->setMaximum(100);
    progress_bar->setMinimum(0);

    tester_id_box->setPrefix("0x");
    ecu_id_box->setPrefix("0x");

    tester_id_box->setRange(0x000, 0xfff);
    ecu_id_box->setRange(0x000, 0xfff);
    tester_id_box->setDisplayIntegerBase(16);
    ecu_id_box->setDisplayIntegerBase(16);

    config_checker_id->setDisplayIntegerBase(16);
    config_checker_id->setRange(0x000, 0xFFF);
    config_checker_id->setPrefix("0x");

    // Storing widgets

    DEBUG(info, "Storing widgets");

    m_filename_label = filename_label;
    m_crc_label = crc_label;
    m_size_label = size_label;
    m_addr_label = addr_label;
    m_progress_bar = progress_bar;
    m_log_frames = log_frames;
    m_log_messages = log_messages;
    m_disconnect_device_button = device_disconnect_btn;
    m_connect_device_button = device_connect_btn;
    m_device_list = devices_list;
    m_start_task_buttons = tasks_btns;
    m_tester_id_box = tester_id_box;
    m_ecu_id_box = ecu_id_box;
    m_plugin_list = plugins_list;
    m_bitrate_list = bitrate_list;
    m_logger_worker =
        new QLoggerWorker(this, log_frames, log_messages, m_progress_bar);
    m_logger = new QLogger(m_logger_worker);
    m_settings_window = settings_window;
    m_mask02 = mask02_box;
    m_mask03 = mask03_box;
    m_config_security_checkbox = config_security_checkbox;
    m_config_postfix = config_postfix;
    m_config_checker_id = config_checker_id;

    // Filling widgets

    int tester_id = m_settings.value("task/testerId").toInt();
    int ecu_id = m_settings.value("task/ecuId").toInt();

    if (ecu_id == 0) ecu_id = 0x76e;
    if (tester_id == 0) tester_id = 0x74e;

    tester_id_box->setValue(tester_id);
    ecu_id_box->setValue(ecu_id);

    bitrate_list->addItem("125000");
    bitrate_list->addItem("250000");
    bitrate_list->addItem("500000");

    device_disconnect_btn->setDisabled(true);

    QString bitrate_last = m_settings.value("device/bitrate").toString();
    int bitrate_id = bitrate_list->findText(bitrate_last);
    if (bitrate_id != -1) bitrate_list->setCurrentIndex(bitrate_id);

    m_mask02->setText(m_settings.value("crypto/mask02").toString());
    m_mask03->setText(m_settings.value("crypto/mask03").toString());
    m_config_security_checkbox->setChecked(
        m_settings.value("settings/security").toInt());
    m_config_postfix->setText(m_settings.value("settings/postfix").toString());
    m_config_checker_id->setValue(
        m_settings.value("settings/checker_id").toUInt());

    // Setting up events

    for(auto btn : tasks_btns) {
        connect(btn, &QPushButton::released, [this, btn]() {
            this->start_task(btn->text());
        });
    }

    connect(m_file_menu_act, &QAction::triggered, this,
            &MainWindow::choose_file);
    connect(m_mask02, &QLineEdit::textChanged, [this](const QString& value) {
        if(value.length() <= 2) {
            return;
        }
        bool ok;
        m_settings.setValue("crypto/mask02", value);
        uint32_t mask = value.toUInt(&ok, 0);
        if(!ok) {
            m_logger->warning("Invalid Mask02 format");
        } else {
            Crypto::SecuritySettings::set_mask02(mask);
        }
    });
    connect(m_mask03, &QLineEdit::textChanged, [this](const QString& value) {
        if(value.length() <= 2) {
            return;
        }
        bool ok;
        m_settings.setValue("crypto/mask03", value);
        uint32_t mask = value.toUInt(&ok, 0);
        if(!ok) {
            m_logger->warning("Invalid Mask03 format");
        } else {
            Crypto::SecuritySettings::set_mask03(mask);
        }
    });
    connect(m_settings_menu_act, &QAction::triggered, this, [this]() {

        m_settings_window->show();
        m_settings_window->setFocus();
    });
    connect(plugins_list, QOverload<const QString&>::of(&QComboBox::activated), [&](QString const&) { update_device_list(); });
    connect(checker_btn, &QPushButton::released, this, &MainWindow::check_devices);
    connect(refresh_btn, &QPushButton::released, [&]() { update_device_list(); });
    connect(device_connect_btn, &QPushButton::released, this,
            &MainWindow::connect_device);
    connect(connect_shortcut, &QShortcut::activated, this,
            &MainWindow::connect_device);
    connect(device_disconnect_btn, &QPushButton::released, this,
            &MainWindow::disconnect_device);
    connect(disconnect_shortcut, &QShortcut::activated, this,
            &MainWindow::disconnect_device);

    // Updating plugins and devices list

    for (std::pair<std::string, std::string> plugin :
         std::vector<std::pair<std::string, std::string>>(CAN_PLUGINS)) {
        if(!QCanBus::instance()->plugins().contains(QString::fromStdString(plugin.second)))
            continue;
        QString errorString;
        QList<QCanBusDeviceInfo> devices =
            QCanBus::instance()->availableDevices(
                QString::fromStdString(plugin.second), &errorString);
        if (!errorString.isEmpty()) {
            // m_logger->error("Error while loading " + plugin.first + ": " + errorString.toStdString());
            m_connect_device_button->setDisabled(true);
        } else {
            m_plugin_list->addItem(QString::fromStdString(plugin.first),
                                   QString::fromStdString(plugin.second));
	    if(devices.size() > 0) {
	        m_plugin_list->setCurrentText(QString::fromStdString(plugin.first));
	    }
        }
    }
    update_device_list();

    DEBUG(info, "Layout created");
}

void MainWindow::update_device_list() {
    m_device_list->clear();
    QString errorString;
    QList<QCanBusDeviceInfo> devices = QCanBus::instance()->availableDevices(
        m_plugin_list->currentData().toString(), &errorString);
    if (!errorString.isEmpty()) {
        m_logger->error(errorString.toStdString());
        m_connect_device_button->setDisabled(true);
    } else {
        for (auto device : devices) {
            m_device_list->addItem(
                device.name() + " (" + device.description() + ")",
                device.name());
        }
        if (devices.size() > 0 && m_device == nullptr) {
            m_connect_device_button->setEnabled(true);
        } else {
            m_connect_device_button->setDisabled(true);
        }
    }
}

void MainWindow::choose_file() {
    DEBUG(info, "Choosing file");
    QString path = m_settings.value("task/file").toString();
    m_file = QFileDialog::getOpenFileName(this, tr("Open HEX"), path,
                                          tr("Intel HEX file (*.hex)"))
                 .toUtf8()
                 .toStdString();
    std::ifstream fin(FILEPATH(m_file));
    
    if (!fin) {
        m_logger->error("Cannot read file. Check if you have permissions.");
        return;
    }
    Hex::HexInfo info;
    Hex::HexReader reader(std::make_shared<Hex::FileSource>(fin));
    DEBUG(info, "Reading file");
    auto maybe_info = Hex::read_hex_info(reader);
    if(!maybe_info) {
        m_logger->error("Failed to read HEX file info");
        return;
    }
    info = maybe_info.value();
    fin.close();
    DEBUG(info, "File readed and closed");
    m_logger->info("Reading file " + m_file);
    m_filename_label->setText(QString::fromStdString("File: " + m_file));
    {
        QString filename = QFileInfo(QString::fromStdString(m_file)).fileName();
        m_filename_label->setText("File: " + filename.toUpper());
    }
    {
        std::stringstream ss;
        ss << "CRC: 0x" << std::setfill('0') << std::setw(4) << std::hex
           << info.crc;
        m_crc_label->setText(QString::fromStdString(ss.str()));
    }
    {
        std::stringstream ss;
        ss << "Size: " << info.size;
        ss << " (" << std::hex << "0x" << info.size << ")";
        m_size_label->setText(QString::fromStdString(ss.str()));
    }
    {
        std::stringstream ss;
        ss << "Start address: 0x" << std::setfill('0') << std::setw(8)
           << std::hex << info.start_addr;
        m_addr_label->setText(QString::fromStdString(ss.str()));
    }
    m_settings.setValue("task/file", QString::fromStdString(m_file));
}

void MainWindow::disconnect_device() {
    if (m_device != nullptr) {
        // disconnect(m_device, &QCanBusDevice::framesReceived, this,
        //            &MainWindow::processReceivedFrames);
        m_device->disconnectDevice();
    }
}

QList<QCanBusDevice::Filter> get_filters(uint32_t ecu_id) {
    QCanBusDevice::Filter filter;
    QList<QCanBusDevice::Filter> filters;
    filter.frameId = ecu_id;
    filter.frameIdMask = 0x000007ff;
    filter.format = QCanBusDevice::Filter::MatchBaseFormat;
    filter.type = QCanBusFrame::DataFrame;
    filters.append(filter);
    return filters;
}

void MainWindow::connect_device_impl(const QString &device_name, std::uint32_t ecu_id, int bitrate) {
    DEBUG(info, "Connecting device");
    QString errorString;
    m_device = QCanBus::instance()->createDevice(
        m_plugin_list->currentData().toString(), device_name, &errorString);
    if (!m_device) {
        m_logger->error(errorString.toStdString());
        m_device = nullptr;
        m_logger->error("Cannot connect device");
        return;
    } else {
        m_logger->info("Connecting " + device_name.toStdString());
        m_tester_id = m_tester_id_box->value();
        m_ecu_id = ecu_id;

        if (m_device->configurationKeys().contains(QCanBusDevice::ConfigurationKey::BitRateKey)) {
            m_device->setConfigurationParameter(
                QCanBusDevice::ConfigurationKey::BitRateKey, bitrate);
        }
        m_device->setConfigurationParameter(QCanBusDevice::RawFilterKey,
                                            QVariant::fromValue(get_filters(m_ecu_id)));

        connect(m_device, &QCanBusDevice::errorOccurred, this, &MainWindow::device_error);
        connect(m_device, &QCanBusDevice::stateChanged, this,
                &MainWindow::device_state_changes);
        connect(m_device, &QCanBusDevice::framesReceived, this,
                &MainWindow::processReceivedFrames);
        if (!m_device->connectDevice()) {
            disconnect(m_device, &QCanBusDevice::errorOccurred, this, &MainWindow::device_error);
            disconnect(m_device, &QCanBusDevice::stateChanged, this,
                    &MainWindow::device_state_changes);
            disconnect(m_device, &QCanBusDevice::framesReceived, this,
                    &MainWindow::processReceivedFrames);
            m_logger->error("Cannot connect device");
        }
    }
}

void MainWindow::connect_device() {
    connect_device_impl(m_device_list->currentData().toString(),
                        m_ecu_id_box->value(),
                        m_bitrate_list->currentText().toInt());
}

void MainWindow::device_state_changes(QCanBusDevice::CanBusDeviceState state) {
    if (state == QCanBusDevice::CanBusDeviceState::ConnectedState) {
        m_disconnect_device_button->setEnabled(true);
        m_connect_device_button->setDisabled(true);
        for (auto btn : m_start_task_buttons) {
            btn->setEnabled(true);
        }
        m_logger->success("Device successfuly connected");
    } else if (state == QCanBusDevice::CanBusDeviceState::UnconnectedState) {
        m_logger->info("Device disconnected");
        m_disconnect_device_button->setDisabled(true);
        m_connect_device_button->setEnabled(true);
        for (auto btn : m_start_task_buttons) {
            if(btn->text() != "Configuration") {
                btn->setDisabled(true);
            }
        }
        disconnect(m_device, &QCanBusDevice::stateChanged, this, &MainWindow::device_state_changes);
        disconnect(m_device, &QCanBusDevice::errorOccurred, this, &MainWindow::device_error);
        disconnect(m_device, &QCanBusDevice::framesReceived, this,
                   &MainWindow::processReceivedFrames);
        m_device = nullptr;
        if(m_checker == nullptr) {
            update_device_list();
        } else {
            emit checker_next();
        }
    }
}

void MainWindow::device_error(QCanBusDevice::CanBusError err) {
    disconnect_device();
}

void MainWindow::abort_task() { emit set_task(nullptr); }

void MainWindow::start_task(QString task_name) {
    m_log_frames->clear();
    m_log_messages->clear();
    for(auto btn : m_start_task_buttons) {
        btn->setDisabled(true);
    }
    m_logger->progress(0);
    bool security = m_config_security_checkbox->isChecked();
    if (task_name == "Flash") {
        m_disconnect_device_button->setDisabled(true);
        if (m_device == nullptr) {
            m_logger->warning("Choose device first");
            return;
        }
        DEBUG(info, "Starting Flash task");
        m_logger->info("Starting task " + task_name.toStdString());
        emit set_task(std::make_shared<FlashTask>(
            m_file, security, std::make_shared<QLogger>(m_logger_worker)));
    } else if (task_name == "Configuration") {
        m_logger->info("Starting task " + task_name.toStdString());
        ConfigurationWindow* window =  new ConfigurationWindow(nullptr);
        emit set_task(std::make_shared<ConfigurationTask>(
            std::make_shared<QLogger>(m_logger_worker),
            security, window, m_device == nullptr));
        window->show();
    }
}

void MainWindow::task_done() {
    for (auto btn : m_start_task_buttons) {
        if (m_device != nullptr || btn->text() == "Configuration") {
            btn->setEnabled(true);
        }
    }
    if (m_device != nullptr) {
        m_disconnect_device_button->setEnabled(true);
    }
    update_device_list();
}

void MainWindow::check_frames_to_write(
    std::shared_ptr<Can::Frame::Frame> frame) {
    if (m_device == nullptr) {
        m_logger->error("No device connected. Cannot send frames");
        return;
    }
    auto maybe_payload = frame->dump();
    if(!maybe_payload) {
        m_logger->error("Invalid frame passed to send. IT SHOULD NOT HAPPEN!");
        return;
    }
    std::vector<uint8_t> payload = maybe_payload.value();
    QCanBusFrame qframe;
    qframe.setFrameId(m_tester_id);
    qframe.setPayload(QByteArray(reinterpret_cast<const char*>(payload.data()),
                                 payload.size()));
    bool res = m_device->writeFrame(qframe);
    if(!res) {
        m_logger->error("Failed to write frame on device");
    }
}

void MainWindow::processReceivedFrames() {
    if(m_device == nullptr) {
        m_logger->error("No devices connected. Cannot receive frames");
        return;
    }

    while (m_device != nullptr && m_device->framesAvailable()) {
        QCanBusFrame qframe = m_device->readFrame();
        if (!qframe.isValid()) continue;
        if (qframe.frameId() == m_ecu_id) {
	    if (m_checker != nullptr) {
                emit frame_received_checker();
		continue;
            }
            QByteArray payload = qframe.payload();
            if (payload.size() < 8) continue;
            auto maybe_frame = Can::Frame::Factory(
                std::vector<uint8_t>(payload.begin(), payload.end())).get();
            if(!maybe_frame) {
                m_logger->error("Cannot parse received frame");
                continue;
            }
            DEBUG(info, "pushing frame to communicator");
            emit frame_received(maybe_frame.value());
        }
    }
}

void MainWindow::check_devices() {
    update_device_list();
    QString errorString;
    QList<QCanBusDeviceInfo> devices = QCanBus::instance()->availableDevices(
        m_plugin_list->currentData().toString(), &errorString);
    QList<int> bitrates;
    for(int i = 0; i < m_bitrate_list->count(); ++i) {
        bitrates.push_back(m_bitrate_list->itemText(i).toInt());
    }
    if (errorString.isEmpty()) {
        m_logger->info("Start searching active devices");
        m_checker = new DeviceCheker(devices, bitrates);
        connect(this, &MainWindow::frame_received_checker, m_checker,
                &DeviceCheker::frame_recieved);
        connect(this, &MainWindow::checker_next, m_checker,
                &DeviceCheker::next);
        connect(m_checker, &DeviceCheker::connect_device, this,
                &MainWindow::connect_device_checker);
        connect(m_checker, &DeviceCheker::device_status, this,
                &MainWindow::device_status_checker);
        connect(m_checker, &DeviceCheker::done, this,
                &MainWindow::checker_done);
        emit checker_next();
    } else {
        m_logger->error("Searching failed");
    }

}

void MainWindow::device_status_checker(bool active, QString const& device_name,
                                       int bitrate) {
    QString bitrate_str = QString::number(bitrate);
    std::string bitrate_stdstr = bitrate_str.toStdString();
    std::string device_name_stdstr = device_name.toStdString();
    if (active) {
        m_logger->success("Device " + device_name.toStdString() +
                          " active with bitrate " + bitrate_str.toStdString());
        m_bitrate_list->setCurrentText(bitrate_str);
        int device_id = m_device_list->findData(device_name);
        m_device_list->setCurrentIndex(device_id);

    } else {
        m_logger->error("Device " + device_name.toStdString() +
                        " inactive with bitrate " +
                        QString::number(bitrate).toStdString());
    }
    disconnect_device();
}

void MainWindow::connect_device_checker(QString const& device_name, int bitrate) {
    connect_device_impl(device_name, m_config_checker_id->value(), bitrate);
}

void MainWindow::checker_done() {
    disconnect(this, &MainWindow::frame_received_checker, m_checker,
               &DeviceCheker::frame_recieved);
    disconnect(this, &MainWindow::checker_next, m_checker, &DeviceCheker::next);
    disconnect(m_checker, &DeviceCheker::connect_device, this,
               &MainWindow::connect_device_checker);
    disconnect(m_checker, &DeviceCheker::device_status, this,
               &MainWindow::device_status_checker);
    disconnect(m_checker, &DeviceCheker::done, this, &MainWindow::checker_done);

    m_logger->info("Searching done");

    delete m_checker;
    m_checker = nullptr;
}
