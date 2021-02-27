#include "mainwindow.h"

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
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>
#include <experimental/filesystem>

#include "communicator.h"
#include "flash.h"
#include "frame.h"
#include "hex.h"
#include "logger.h"
#include "qcommunicator.h"
#include "qtask.h"
#include "task.h"
#include "security.h"

#ifdef __MINGW32__
#define CAN_PLUGINS                                         \
    {                                                       \
        {"sysWORXX", "systeccan"}, {"IXXAT", "ixxatcan"}, { \
            "VECTOR", "vectorcan"                           \
        }                                                   \
    }
#elif __linux__
#define CAN_PLUGINS                  \
    {                                \
        { "SocketCAN", "socketcan" } \
    }
#endif

namespace fs = std::experimental::filesystem;

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_device(),
      m_settings("canFlash", "Some cool organization name") {
    QWidget* window = new QWidget();

    create_layout(window);

    bool ok;
    uint32_t mask = m_settings.value("crypto/mask02").toString().toUInt(&ok, 0);
    if(ok) Crypto::SecuritySettings::set_mask02(mask);
    else {
        m_settings.setValue("crypto/mask02", Crypto::SecuritySettings::get_mask02());
    }
    mask = m_settings.value("crypto/mask03").toString().toUInt(&ok, 0);
    if(ok) Crypto::SecuritySettings::set_mask03(mask);
    else {
        m_settings.setValue("crypto/mask03", Crypto::SecuritySettings::get_mask03());
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
    QGroupBox* devices_buttons_group = new QGroupBox();
    QGroupBox* tasks_group = new QGroupBox(tr("Tasks"));
    QGroupBox* tasks_buttons_group = new QGroupBox();
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
    QVBoxLayout* tasks_group_layout = new QVBoxLayout(tasks_group);
    QHBoxLayout* tasks_buttons_layout = new QHBoxLayout(tasks_buttons_group);
    QHBoxLayout* tester_id_layout = new QHBoxLayout(tester_id_frame);
    QHBoxLayout* ecu_id_layout = new QHBoxLayout(ecu_id_frame);

    QTextEdit* log_frames = new QTextEdit();
    QTextEdit* log_messages = new QTextEdit();

    QProgressBar* progress_bar = new QProgressBar();

    QComboBox* plugins_list = new QComboBox();
    QComboBox* devices_list = new QComboBox(devices_group);
    QComboBox* bitrate_list = new QComboBox(devices_group);
    QComboBox* tasks_list = new QComboBox(tasks_group);

    QPushButton* device_connect_btn = new QPushButton("Connect");
    QPushButton* device_disconnect_btn = new QPushButton("Disconnect");
    QPushButton* task_start_btn = new QPushButton("Start");

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

    QVBoxLayout* settings_window_layout = new QVBoxLayout(settings_window);
    QHBoxLayout* settings_mask02_layout = new QHBoxLayout(settings_mask02_frame);
    QHBoxLayout* settings_mask03_layout = new QHBoxLayout(settings_mask03_frame);

    QLineEdit* mask02_box = new QLineEdit();
    QLineEdit* mask03_box = new QLineEdit();

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
    devices_group_layout->addWidget(devices_buttons_group);

    devices_buttons_layout->addWidget(device_connect_btn);
    devices_buttons_layout->addWidget(device_disconnect_btn);

    tester_id_layout->addWidget(new QLabel("Tester:"));
    tester_id_layout->addWidget(tester_id_box);

    ecu_id_layout->addWidget(new QLabel("ECU:"));
    ecu_id_layout->addWidget(ecu_id_box);

    tasks_group_layout->addWidget(tester_id_frame);
    tasks_group_layout->addWidget(ecu_id_frame);
    tasks_group_layout->addWidget(tasks_list);
    tasks_group_layout->addWidget(tasks_buttons_group);

    tasks_buttons_layout->addWidget(task_start_btn);

    settings_window_layout->addWidget(settings_mask02_frame);
    settings_window_layout->addWidget(settings_mask03_frame);

    settings_mask02_layout->addWidget(new QLabel("MASK02"));
    settings_mask02_layout->addWidget(mask02_box);
    settings_mask03_layout->addWidget(new QLabel("MASK03"));
    settings_mask03_layout->addWidget(mask03_box);

    // Setting up widgets

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

    // Storting widgets

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
    m_start_task_button = task_start_btn;
    m_task_list = tasks_list;
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

    // Filling widgets

    int tester_id = m_settings.value("task/testerId").toInt();
    int ecu_id = m_settings.value("task/ecuId").toInt();

    if (ecu_id == 0) ecu_id = 0x76e;
    if (tester_id == 0) tester_id = 0x74e;

    tester_id_box->setValue(tester_id);
    ecu_id_box->setValue(ecu_id);

    tasks_list->addItem("Flash");
    tasks_list->addItem("Test");

    bitrate_list->addItem("125000");
    bitrate_list->addItem("250000");
    bitrate_list->addItem("500000");

    device_disconnect_btn->setDisabled(true);

    QString bitrate_last = m_settings.value("device/bitrate").toString();
    int bitrate_id = bitrate_list->findText(bitrate_last);
    if (bitrate_id != -1) bitrate_list->setCurrentIndex(bitrate_id);

    auto update_device_list = [this](const QString& str) {
        m_device_list->clear();
        QString errorString;
        QList<QCanBusDeviceInfo> devices =
            QCanBus::instance()->availableDevices(
                m_plugin_list->currentData().toString(), &errorString);
        if (!errorString.isEmpty()) {
            m_logger->error(errorString.toStdString());
        } else {
            for (auto device : devices) {
                m_device_list->addItem(
                    device.name() + " (" + device.description() + ")",
                    device.name());
            }
        }
    };


    for (std::pair<std::string, std::string> plugin :
         std::vector<std::pair<std::string, std::string>>(CAN_PLUGINS)) {
        if(!QCanBus::instance()->plugins().contains(QString::fromStdString(plugin.second)))
            continue;
        QString errorString;
        QList<QCanBusDeviceInfo> devices =
            QCanBus::instance()->availableDevices(
                QString::fromStdString(plugin.second), &errorString);
        if (!errorString.isEmpty()) {
            m_logger->error("Error while loading " + plugin.first);
        } else {
            m_plugin_list->addItem(QString::fromStdString(plugin.first),
                                   QString::fromStdString(plugin.second));
            m_plugin_list->setCurrentText(QString::fromStdString(plugin.first));
            if (devices.size() == 0) continue;
            update_device_list(QString::fromStdString(plugin.first));
        }
    }

    // Setting up events

    connect(bitrate_list, &QComboBox::currentTextChanged, [this]() {
        m_settings.setValue("device/bitrate", m_bitrate_list->currentText());
    });

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
    connect(m_file_menu_act, &QAction::triggered, this,
            &MainWindow::choose_file);
    connect(m_settings_menu_act, &QAction::triggered, this, [this]() {
        m_mask02->setText(m_settings.value("crypto/mask02").toString());
        m_mask03->setText(m_settings.value("crypto/mask03").toString());

        m_settings_window->show();
        m_settings_window->setFocus();
    });
    connect(plugins_list, QOverload<const QString&>::of(&QComboBox::activated), update_device_list);
    connect(device_connect_btn, &QPushButton::released, this,
            &MainWindow::connect_device);
    connect(device_disconnect_btn, &QPushButton::released, this,
            &MainWindow::disconnect_device);
    connect(tester_id_box, QOverload<int>::of(&QSpinBox::valueChanged),
            [&](int v) { m_settings.setValue("task/testerId", v); });
    connect(ecu_id_box, QOverload<int>::of(&QSpinBox::valueChanged),
            [&](int v) { m_settings.setValue("task/ecuId", v); });
    connect(task_start_btn, &QPushButton::released, this,
            &MainWindow::start_task);

    DEBUG(info, "Layout created");
}

void MainWindow::choose_file() {
    DEBUG(info, "Choosing file");
    QString path = m_settings.value("task/file").toString();
    m_file = QFileDialog::getOpenFileName(this, tr("Open HEX"), path,
                                          tr("Intel HEX file (*.hex)"))
                 .toUtf8()
                 .toStdString();
#ifdef __MINGW32__
    std::ifstream fin(fs::u8path(m_file).wstring().c_str());
#else
    std::ifstream fin(m_file);
#endif
    
    if (!fin) {
        m_logger->error("Cannot read file. Check if you have permissions.");
        return;
    }
    Hex::HexInfo info;
    try {
        Hex::HexReader reader(std::make_shared<Hex::FileSource>(fin));
        DEBUG(info, "Reading file");
        info = Hex::read_hex_info(reader);
    } catch (std::runtime_error e) {
        m_logger->error(e.what());
    }
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
        disconnect(m_device, &QCanBusDevice::framesReceived, this,
                   &MainWindow::processReceivedFrames);
        m_device->disconnectDevice();
        delete m_device;
        m_logger->info("Device disconnected");
        m_disconnect_device_button->setDisabled(true);
        m_connect_device_button->setEnabled(true);
    }
}

void MainWindow::connect_device() {
    DEBUG(info, "Connecting device");
    QString errorString;
    QCanBusDevice::Filter filter;
    QList<QCanBusDevice::Filter> filters;
    m_tester_id = m_tester_id_box->value();
    m_ecu_id = m_ecu_id_box->value();
    filter.frameId = m_ecu_id;
    filter.frameIdMask = 0x000007ff;
    filter.format = QCanBusDevice::Filter::MatchBaseFormat;
    filter.type = QCanBusFrame::DataFrame;
    filters.append(filter);
    QString device_name = m_device_list->currentData().toString();
    m_device = QCanBus::instance()->createDevice(
        m_plugin_list->currentData().toString(), device_name, &errorString);
    if (!m_device) {
        m_logger->error(errorString.toStdString());
        delete m_device;
        m_device = nullptr;
        m_logger->error("Cannot connect device");
        return;
    } else {
        m_logger->info("Connecting " + device_name.toStdString());
        int bitrate = m_bitrate_list->currentText().toInt();
        m_device->setConfigurationParameter(
            QCanBusDevice::ConfigurationKey::BitRateKey, bitrate);
        m_device->setConfigurationParameter(QCanBusDevice::RawFilterKey,
                                            QVariant::fromValue(filters));
        if (m_device->connectDevice()) {
            connect(m_device, &QCanBusDevice::framesReceived, this,
                    &MainWindow::processReceivedFrames);
            m_logger->info(device_name.toStdString() +
                           " successfuly connected");
            DEBUG(info, "Device connected");
            m_disconnect_device_button->setEnabled(true);
            m_connect_device_button->setDisabled(true);
        } else {
            m_logger->error("Cannot connect device");
            delete m_device;
            m_device = nullptr;
        }
    }
}

void MainWindow::abort_task() { emit set_task(nullptr); }

void MainWindow::start_task() {
    if (m_device == nullptr) {
        m_logger->warning("Choose device first");
        return;
    }
    m_log_frames->clear();
    m_log_messages->clear();
    QString task_name = m_task_list->currentText();
    m_start_task_button->setDisabled(true);
    m_disconnect_device_button->setDisabled(true);
    m_logger->progress(0);
    // m_abort_task_button->setEnabled(true);
    if (task_name == "Flash") {
        DEBUG(info, "Starting Flash task");
        m_logger->info("Starting task " + task_name.toStdString());
        emit set_task(std::make_shared<FlashTask>(
            m_file, std::make_shared<QLogger>(m_logger_worker)));
    } else if (task_name == "Test") {
        m_logger->info("Starting task " + task_name.toStdString());
        emit set_task(std::make_shared<QTestTask>(
            std::make_shared<QLogger>(m_logger_worker)));
    }
}

void MainWindow::task_done() {
    m_start_task_button->setEnabled(true);
    m_disconnect_device_button->setEnabled(true);
    // m_abort_task_button->setDisabled(true);
}

void MainWindow::check_frames_to_write(std::shared_ptr<Can::Frame> frame) {
    std::vector<uint8_t> payload = frame->dump();
    QCanBusFrame qframe;
    qframe.setFrameId(m_tester_id);
    qframe.setPayload(QByteArray(reinterpret_cast<const char*>(payload.data()),
                                 payload.size()));
    bool res = m_device->writeFrame(qframe);
}

void MainWindow::processReceivedFrames() {
    std::unique_lock<std::mutex> lock(m_communicator_mutex);
    while (m_device->framesAvailable()) {
        QCanBusFrame qframe = m_device->readFrame();
        if (!qframe.isValid()) continue;
        if (qframe.frameId() == m_ecu_id) {
            QByteArray payload = qframe.payload();
            if (payload.size() < 8) continue;
            std::shared_ptr<Can::Frame> frame =
                std::move(Can::FrameFactory(std::vector<uint8_t>(
                                                payload.begin(), payload.end()))
                              .get());
            DEBUG(info, "pushing frame to cmmunicator");
            // m_communicator->push_frame(frame);
            emit frame_received(frame);
        }
    }
}

MainWindow::~MainWindow() {
    m_communicator_thread.terminate();
    m_communicator_thread.wait();
    if (m_device != nullptr) delete m_device;
    if (m_communicator != nullptr) delete m_communicator;
}
