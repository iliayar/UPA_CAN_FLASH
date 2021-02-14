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
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QTextEdit>
#include <QThread>
#include <QVBoxLayout>
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

#ifdef __MINGW32__
#define CAN_PLUGINS \
    { "systeccan", "ixxatcan" }
#elif __linux__
#define CAN_PLUGINS \
    { "socketcan" }
#endif

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
      m_device(),
      m_settings("canFlash", "Some cool organization name") {
    QWidget* window = new QWidget();

    create_layout(window);

    m_device = nullptr;
    m_communicator = nullptr;
    m_communicator_thread.start();

    setCentralWidget(window);
}
void MainWindow::create_layout(QWidget* root) {
    DEBUG(info, "Creating layout");
    // Actions

    m_file_menu_act = new QAction(tr("&Choose"), this);
    connect(m_file_menu_act, &QAction::triggered, this,
            &MainWindow::choose_file);

    // Menu
    QMenu* file_menu = new QMenu(tr("File"));
    file_menu->addAction(m_file_menu_act);
    menuBar()->addMenu(file_menu);

    // Main layout

    QHBoxLayout* main_layout = new QHBoxLayout(root);
    QGroupBox* log_group = new QGroupBox(tr("Logs"));
    QGroupBox* options_group = new QGroupBox(tr("Options"));

    main_layout->addWidget(log_group, 70);
    main_layout->addWidget(options_group, 30);

    // Log layout

    const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);

    QHBoxLayout* log_layout = new QHBoxLayout(log_group);
    QTextEdit* log_frames = new QTextEdit();
    log_frames->setReadOnly(true);
    log_frames->setFont(fixedFont);
    QTextEdit* log_messages = new QTextEdit();
    log_messages->setReadOnly(true);
    log_messages->setFont(fixedFont);

    log_layout->addWidget(log_messages);
    log_layout->addWidget(log_frames);

    m_logger_worker = new QLoggerWorker(this, log_frames, log_messages);
    m_logger = new QLogger(m_logger_worker);

    m_log_frames = log_frames;
    m_log_messages = log_messages;

    // Options layout
    QVBoxLayout* options_layout = new QVBoxLayout(options_group);
    //   file options layout
    QComboBox* plugins_list = new QComboBox();
    for (std::string plugin : std::vector<std::string>(CAN_PLUGINS)) {
        plugins_list->addItem(QString::fromStdString(plugin));
    }
    QGroupBox* file_group = new QGroupBox(tr("File"));

    options_layout->addWidget(plugins_list);
    options_layout->addWidget(file_group);

    QVBoxLayout* file_layout = new QVBoxLayout(file_group);
    m_filename_label = new QLabel("Choose file..");
    m_crc_label = new QLabel("CRC: ???");
    m_size_label = new QLabel("Size: ???");
    m_addr_label = new QLabel("Begin address: ???");

    // file_layout->addWidget(m_filename_label);
    file_layout->addWidget(m_crc_label);
    file_layout->addWidget(m_size_label);
    file_layout->addWidget(m_addr_label);

    auto update_device_list = [=](const QString& str) {
        m_device_list->clear();
        QString errorString;
        QList<QCanBusDeviceInfo> devices =
            QCanBus::instance()->availableDevices(m_plugin_list->currentText(),
                                                  &errorString);
        if (!errorString.isEmpty()) {
            m_logger->error(errorString.toStdString());
        } else {
            for (auto device : devices) {
                m_device_list->addItem(device.name());
            }
        }
    };

    connect(plugins_list, &QComboBox::textActivated, update_device_list);

    //   devices options layout
    QGroupBox* devices_group = new QGroupBox(tr("Devices"));

    options_layout->addWidget(devices_group);

    QVBoxLayout* devices_group_layout = new QVBoxLayout(devices_group);
    QGroupBox* devices_buttons_group = new QGroupBox();
    QHBoxLayout* devices_buttons_layout =
        new QHBoxLayout(devices_buttons_group);

    QComboBox* devices_list = new QComboBox(devices_group);
    QComboBox* bitrate_list = new QComboBox(devices_group);

    devices_group_layout->addWidget(devices_list);
    devices_group_layout->addWidget(bitrate_list);
    devices_group_layout->addWidget(devices_buttons_group);

    QPushButton* device_connect_btn = new QPushButton("Connect");
    QPushButton* device_disconnect_btn = new QPushButton("Disconnect");
    devices_buttons_layout->addWidget(device_connect_btn);
    devices_buttons_layout->addWidget(device_disconnect_btn);

    m_device_list = devices_list;
    connect(device_connect_btn, &QPushButton::released, this,
            &MainWindow::connect_device);
    bitrate_list->addItem("125000");
    bitrate_list->addItem("250000");
    bitrate_list->addItem("500000");
    QString bitrate_last = m_settings.value("device/bitrate").toString();
    int bitrate_id = bitrate_list->findText(bitrate_last);
    if (bitrate_id != -1) bitrate_list->setCurrentIndex(bitrate_id);
    m_bitrate_list = bitrate_list;
    connect(m_bitrate_list, &QComboBox::currentTextChanged, [&]() {
        m_settings.setValue("device/bitrate", m_bitrate_list->currentText());
    });
    //   tasks options layout

    QGroupBox* tasks_group = new QGroupBox(tr("Tasks"));

    options_layout->addWidget(tasks_group);

    QVBoxLayout* tasks_group_layout = new QVBoxLayout(tasks_group);
    QGroupBox* tasks_buttons_group = new QGroupBox();
    QHBoxLayout* tasks_buttons_layout = new QHBoxLayout(tasks_buttons_group);
    QComboBox* tasks_list = new QComboBox(tasks_group);

    QSpinBox* tester_id_box = new QSpinBox();
    QSpinBox* ecu_id_box = new QSpinBox();

    QFrame* tester_id_frame = new QFrame();
    QFrame* ecu_id_frame = new QFrame();

    QHBoxLayout* tester_id_layout = new QHBoxLayout(tester_id_frame);
    QHBoxLayout* ecu_id_layout = new QHBoxLayout(ecu_id_frame);

    tester_id_layout->addWidget(new QLabel("Tester:"));
    ecu_id_layout->addWidget(new QLabel("ECU:"));
    tester_id_layout->addWidget(tester_id_box);
    ecu_id_layout->addWidget(ecu_id_box);
    tester_id_box->setPrefix("0x");
    ecu_id_box->setPrefix("0x");

    QString tester_id_str = m_settings.value("task/testerId").toString();
    QString ecu_id_str = m_settings.value("task/ecuId").toString();
    int tester_id =
        tester_id_str.right(tester_id_str.size() - 2).toLong(nullptr, 16);
    int ecu_id = ecu_id_str.right(ecu_id_str.size() - 2).toLong(nullptr, 16);

    tester_id_box->setRange(0x000, 0xfff);
    ecu_id_box->setRange(0x000, 0xfff);
    tester_id_box->setDisplayIntegerBase(16);
    ecu_id_box->setDisplayIntegerBase(16);
    tester_id_box->setValue(tester_id);
    ecu_id_box->setValue(ecu_id);

    tasks_group_layout->addWidget(tester_id_frame);
    tasks_group_layout->addWidget(ecu_id_frame);
    tasks_group_layout->addWidget(tasks_list);
    tasks_group_layout->addWidget(tasks_buttons_group);

    QPushButton* task_start_btn = new QPushButton("Start");
    QPushButton* task_abort_btn = new QPushButton("Abort");
    tasks_buttons_layout->addWidget(task_start_btn);
    tasks_buttons_layout->addWidget(task_abort_btn);

    tasks_list->addItem("Flash");
    tasks_list->addItem("Test");
    m_task_list = tasks_list;

    m_tester_id_box = tester_id_box;
    m_ecu_id_box = ecu_id_box;

    m_plugin_list = plugins_list;

    connect(m_tester_id_box, &QSpinBox::textChanged,
            [&](QString v) { m_settings.setValue("task/testerId", v); });
    connect(m_ecu_id_box, &QSpinBox::textChanged,
            [&](QString v) { m_settings.setValue("task/ecuId", v); });

    connect(task_start_btn, &QPushButton::released, this,
            &MainWindow::start_task);
    DEBUG(info, "Layout created");

    update_device_list(std::vector<QString>(CAN_PLUGINS)[0]);
}

void MainWindow::update_devices_list() {}

void MainWindow::choose_file() {
    DEBUG(info, "Choosing file");
    QString path = m_settings.value("general/file").toString();
    m_file = QFileDialog::getOpenFileName(this, tr("Open HEX"), path,
                                          tr("Intel HEX file (*.hex)"))
                 .toStdString();
    std::ifstream fin(m_file);
    if (!fin) return;
    Hex::HexReader reader(new Hex::FileSource(fin));
    DEBUG(info, "Reading file");
    Hex::HexInfo info = Hex::read_hex_info(reader);
    fin.close();
    DEBUG(info, "File readed and closed");
    m_logger->info("Reading file " + m_file);
    m_filename_label->setText(QString::fromStdString("File: " + m_file));
    {
        std::stringstream ss;
        ss << "CRC: 0x" << std::setfill('0') << std::setw(4) << std::hex
           << info.crc;
        m_crc_label->setText(QString::fromStdString(ss.str()));
    }
    {
        std::stringstream ss;
        ss << "Size: " << info.size;
        m_size_label->setText(QString::fromStdString(ss.str()));
    }
    {
        std::stringstream ss;
        ss << "Start address: 0x" << std::setfill('0') << std::setw(8)
           << std::hex << info.start_addr;
        m_addr_label->setText(QString::fromStdString(ss.str()));
    }
    m_settings.setValue("general/file", QString::fromStdString(m_file));
}

void MainWindow::connect_device() {
    DEBUG(info, "Connecting device");
    QString errorString;
    QCanBusDevice::Filter filter;
    QList<QCanBusDevice::Filter> filters;
    m_tester_id = m_tester_id_box->value();
    m_ecu_id = m_ecu_id_box->value();
    filter.frameId = m_ecu_id;
    filter.format = QCanBusDevice::Filter::MatchBaseFormat;
    filter.type = QCanBusFrame::DataFrame;
    filters.append(filter);
    QString device_name = m_device_list->currentText();
    m_device = QCanBus::instance()->createDevice(m_plugin_list->currentText(),
                                                 device_name, &errorString);
    if (!m_device) {
        m_logger->error(errorString.toStdString());
        delete m_device;
        m_device = nullptr;
        m_logger->error("Cannot connect device");
        return;
    } else {
        m_logger->info("Connecting " + device_name.toStdString());
        m_device->setConfigurationParameter(
            QCanBusDevice::ConfigurationKey::BitRateKey,
            m_bitrate_list->currentText());
        m_device->setConfigurationParameter(QCanBusDevice::RawFilterKey, QVariant::fromValue(filters));
        if (m_device->connectDevice()) {
            connect(m_device, &QCanBusDevice::framesReceived, this,
                    &MainWindow::processReceivedFrames);
            m_communicator = new QCommunicator(new QLogger(m_logger_worker));
            m_logger->info(device_name.toStdString() +
                           " successfuly connected");
            DEBUG(info, "Device connected");
            connect(m_communicator, &QCommunicator::fetch_frame, this,
                    &MainWindow::check_frames_to_write);
            connect(this, &MainWindow::frame_received, m_communicator,
                    &QCommunicator::push_frame);
	    connect(this, &MainWindow::set_task, m_communicator, 
		    &QCommunicator::set_task);
            m_communicator->moveToThread(&m_communicator_thread);
        } else {
            m_logger->error("Cannot connect device");
            delete m_device;
            m_device = nullptr;
        }
    }
}

void MainWindow::start_task() {
    if (m_device == nullptr) {
        m_logger->warning("Choose device first");
        return;
    }
    m_log_frames->clear();
    m_log_messages->clear();
    QString task_name = m_task_list->currentText();
    if (task_name == "Flash") {
        DEBUG(info, "Starting FLash task");
        m_logger->info("Starting task " + task_name.toStdString());
        emit set_task(new FlashTask(m_file, new QLogger(m_logger_worker)));
    } else if (task_name == "Test") {
        m_logger->info("Starting task " + task_name.toStdString());
        emit set_task(new QTestTask(new QLogger(m_logger_worker)));
    }
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
    if (m_device != nullptr) delete m_device;
    if (m_communicator != nullptr) delete m_communicator;
}

void CommunicatorThread::run() {
    while (true) {
        {
            std::unique_lock<std::mutex> lock(m_communicator_mutex);
            try {
                m_communicator->get_status();
                std::shared_ptr<Can::Frame> frame =
                    m_communicator->fetch_frame();
                DEBUG(info, "fetched frame from communicator");
                emit check_frames_to_write(frame);
            } catch (Can::NothingToFetch e) {
            }
        }
        usleep(50);
    }
}
