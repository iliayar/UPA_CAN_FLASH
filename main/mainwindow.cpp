#include "mainwindow.h"

#include <QCanBus>
#include <QCanBusFrame>
#include <QCoreApplication>
#include <QDebug>
#include <QThread>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QTextEdit>
#include <QGroupBox>
#include <QFontDatabase>
#include <QComboBox>
#include <QPushButton>
#include <QMenu>
#include <QMenuBar>
#include <QContextMenuEvent>
#include <QFileDialog>
#include <QLabel>
#include <iostream>
#include <vector>
#include <memory>
#include <sstream>

#include "communicator.h"
#include "frame.h"
#include "task.h"
#include "flash.h"
#include "can.h"
#include "hex.h"

#ifdef __MINGW32__
#define CAN_PLUGIN "systeccan"
#elif __linux__
#define CAN_PLUGIN "socketcan"
#endif

#define DEVICE_ID 0x76E
#define TESTER_ID 0x74E

MainWindow::MainWindow(QWidget* parent) : QMainWindow(parent), m_device() {
    QWidget* window = new QWidget();

    create_layout(window);

    m_device = nullptr;
    m_communicator = nullptr;
    m_communicator_thread = nullptr;

    // std::cout << "Using device " << device_name.toStdString() << std::endl;

    setCentralWidget(window);
}
void MainWindow::create_layout(QWidget* root) {

// Actions

    m_file_menu_act = new QAction(tr("&Choose"), this);
    connect(m_file_menu_act, &QAction::triggered, this, &MainWindow::choose_file);

// Menu
    QMenu* file_menu = new QMenu(tr("File"));
    file_menu->addAction(m_file_menu_act);
    menuBar()->addMenu(file_menu);

// Main layout
    
    QHBoxLayout* main_layout = new QHBoxLayout(root);
    QGroupBox* log_group = new QGroupBox(tr("Logs"));
    QGroupBox* options_group = new QGroupBox(tr("Options"));

    main_layout->addWidget(log_group);
    main_layout->addWidget(options_group);

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

// Options layout
    QVBoxLayout* options_layout = new QVBoxLayout(options_group);
//   file options layout
    QGroupBox* file_group = new QGroupBox(tr("File"));

    options_layout->addWidget(file_group);

    QVBoxLayout* file_layout = new QVBoxLayout(file_group);
    m_filename_label = new QLabel("Choose file..");
    m_crc_label = new QLabel("CRC: ???");
    m_size_label = new QLabel("Size: ???");
    m_addr_label = new QLabel("Begin address: ???");

    file_layout->addWidget(m_filename_label);
    file_layout->addWidget(m_crc_label);
    file_layout->addWidget(m_size_label);
    file_layout->addWidget(m_addr_label);

//   devices options layout
    QGroupBox *devices_group = new QGroupBox(tr("Devices"));

    options_layout->addWidget(devices_group);

    QVBoxLayout* devices_group_layout = new QVBoxLayout(devices_group);
    QGroupBox* devices_buttons_group = new QGroupBox();
    QHBoxLayout* devices_buttons_layout = new QHBoxLayout(devices_buttons_group);


    QComboBox* devices_list = new QComboBox(devices_group);
    QComboBox* bitrate_list = new QComboBox(devices_group);

    devices_group_layout->addWidget(devices_list);
    devices_group_layout->addWidget(bitrate_list);
    devices_group_layout->addWidget(devices_buttons_group);
    
    QPushButton* device_connect_btn = new QPushButton("Connect");
    QPushButton* device_disconnect_btn = new QPushButton("Disconnect");
    devices_buttons_layout->addWidget(device_connect_btn);
    devices_buttons_layout->addWidget(device_disconnect_btn);

    QString errorString;
    QList<QCanBusDeviceInfo> devices = QCanBus::instance()->availableDevices(
        QStringLiteral(CAN_PLUGIN), &errorString);
    if (!errorString.isEmpty()) {
        m_logger->error(errorString.toStdString());
    } else {
        for (auto device : devices) {
            devices_list->addItem(device.name());
        }
    }

    m_device_list = devices_list;
    connect(device_connect_btn, &QPushButton::released, this, &MainWindow::connect_device);
    bitrate_list->addItem("125000");
    bitrate_list->addItem("250000");
    bitrate_list->addItem("500000");
    m_bitrate_list = bitrate_list;
    //   tasks options layout

    QGroupBox* tasks_group = new QGroupBox(tr("Tasks"));

    options_layout->addWidget(tasks_group);

    QVBoxLayout* tasks_group_layout = new QVBoxLayout(tasks_group);
    QGroupBox* tasks_buttons_group = new QGroupBox();
    QHBoxLayout* tasks_buttons_layout = new QHBoxLayout(tasks_buttons_group);
    QComboBox* tasks_list = new QComboBox(tasks_group);
    tasks_group_layout->addWidget(tasks_list);
    tasks_group_layout->addWidget(tasks_buttons_group);
    
    QPushButton* task_start_btn = new QPushButton("Start");
    QPushButton* task_abort_btn = new QPushButton("Abort");
    tasks_buttons_layout->addWidget(task_start_btn);
    tasks_buttons_layout->addWidget(task_abort_btn);

    tasks_list->addItem("Flash");
    tasks_list->addItem("Test");
    m_task_list = tasks_list;

    connect(task_start_btn, &QPushButton::released, this, &MainWindow::start_task);

}

void MainWindow::choose_file() {
    m_file = QFileDialog::getOpenFileName(this, tr("Open HEX"), "./", tr("Intel HEX file (*.hex)")).toStdString();
    std::ifstream fin(m_file);
    if(!fin) return;
    Hex::HexReader reader(new Hex::FileSource(fin));
    Hex::HexInfo info = Hex::read_hex_info(reader);
    fin.close();
    m_logger->info("Reading file " + m_file);
    m_filename_label->setText(QString::fromStdString("File: " + m_file));
    {
        std::stringstream ss;
        ss << "CRC: 0x" << std::setfill('0') << std::setw(4) << std::hex << info.crc;
        m_crc_label->setText(QString::fromStdString(ss.str()));
    }
    {
        std::stringstream ss;
        ss << "Size: " << info.size;
        m_size_label->setText(QString::fromStdString(ss.str()));
    }
    {
        std::stringstream ss;
        ss << "Start address: 0x" << std::setfill('0') << std::setw(8) << std::hex << info.start_addr;
        m_addr_label->setText(QString::fromStdString(ss.str()));
    }
}

void MainWindow::connect_device() {
    QString errorString;
    QString device_name = m_device_list->currentText();
    m_device = QCanBus::instance()->createDevice(QStringLiteral(CAN_PLUGIN),
                                                 device_name, &errorString);
    if (!m_device) {
        m_logger->error( errorString.toStdString() );
        delete m_device;
        m_device = nullptr;
        return;
    } else {
        m_logger->info("Connecting " + device_name.toStdString() );
        m_device->setConfigurationParameter(QCanBusDevice::ConfigurationKey::BitRateKey, m_bitrate_list->currentText());
        if (m_device->connectDevice()) {
            connect(m_device, &QCanBusDevice::framesReceived, this,
                    &MainWindow::processReceivedFrames);
            // m_communicator = new Can::Communicator(new Can::FramesStdLogger());
            m_communicator = new Can::Communicator(new QLogger(m_logger_worker));
            // m_logger = new Can::FrameStdLogger();
            m_logger->info(device_name.toStdString() + " successfuly connected");
            m_communicator_thread = new CommunicatorThread(
                this, m_communicator, m_communicator_mutex);
            connect(m_communicator_thread,
                    &CommunicatorThread::check_frames_to_write, this,
                    &MainWindow::check_frames_to_write);
            m_communicator_thread->start();
        } else {
            m_logger->error( "Cannot connect device" );
            delete m_device;
            m_device = nullptr;
        }
    }
}

void MainWindow::start_task() {
    if(m_device == nullptr) {
        m_logger->warning("Choose device first");
        return;
    }
    QString task_name = m_task_list->currentText();
    if(task_name == "Flash") {
        m_logger->info("Starting task " + task_name.toStdString());
        // m_communicator->set_task(new FlashTask(m_file, new Can::FramesStdLogger()));
        m_communicator->set_task(new FlashTask(m_file, new QLogger(m_logger_worker)));
    } else if(task_name == "Test") {
        m_logger->info("Starting task " + task_name.toStdString());
        m_communicator->set_task(new Can::ReadWriteThreadedTask(new QLogger(m_logger_worker)));
    }

}

void MainWindow::check_frames_to_write(std::shared_ptr<Can::Frame> frame) {
    std::vector<uint8_t> payload = frame->dump();
    QCanBusFrame qframe;
    qframe.setFrameId(TESTER_ID);
    qframe.setPayload(QByteArray(reinterpret_cast<const char*>(payload.data()),
				 payload.size()));
    m_device->writeFrame(qframe);
}

void MainWindow::processReceivedFrames() {
    std::unique_lock<std::mutex> lock(m_communicator_mutex);
    while (m_device->framesAvailable()) {
        QCanBusFrame qframe = m_device->readFrame();
        if (qframe.frameId() == DEVICE_ID) {
            QByteArray payload = qframe.payload();
            std::shared_ptr<Can::Frame> frame =
                std::move(Can::FrameFactory(std::vector<uint8_t>(
                                                payload.begin(), payload.end()))
                              .get());
            m_communicator->push_frame(frame);
        }
    }
}

MainWindow::~MainWindow() {
    if(m_device != nullptr) delete m_device;
    if(m_communicator != nullptr) delete m_communicator;
    if(m_communicator_thread != nullptr) delete m_communicator_thread;
}

void CommunicatorThread::run() {
    while (true) {
        {
            std::unique_lock<std::mutex> lock(m_communicator_mutex);
            try {
                m_communicator->get_status();
                std::shared_ptr<Can::Frame> frame = m_communicator->fetch_frame();
                emit check_frames_to_write(frame);
            } catch (Can::NothingToFetch e) {
            }
        }
    }
}
