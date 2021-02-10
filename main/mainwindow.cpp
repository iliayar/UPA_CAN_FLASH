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
#include <iostream>
#include <vector>
#include <memory>

#include "communicator.h"
#include "frame.h"
#include "task.h"
#include "flash.h"
#include "can.h"

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

    // Listing devices
    // QString errorString;
    // QList<QCanBusDeviceInfo> devices = QCanBus::instance()->availableDevices(
	// QStringLiteral(CAN_PLUGIN), &errorString);
    // if (!errorString.isEmpty()) {
	// std::cerr << errorString.toStdString() << std::endl;
    // } else {
	// std::cout << "Devices" << std::endl;
	// for (auto device : devices) {
	//     std::cout << "- " << device.name().toStdString() << std::endl;
	//     std::cout << "  " << device.description().toStdString()
	// 	      << std::endl;
	// }
    // }

    // if (devices.size() == 0) {
	// std::cerr << "No devices found" << std::endl;
	// exit(1);
    // }
    // QString device_name = devices[0].name();

    // std::cout << "Using device " << device_name.toStdString() << std::endl;
    // m_device = QCanBus::instance()->createDevice(QStringLiteral(CAN_PLUGIN),
	// 					 device_name, &errorString);
    // if (!m_device) {
	// std::cerr << errorString.toStdString() << std::endl;
    // } else {
	// std::cout << "Connecting device" << std::endl;
	// if (m_device->connectDevice()) {
	//     std::cout << m_device->state() << std::endl;
	//     connect(m_device, &QCanBusDevice::framesReceived, this,
	// 	    &MainWindow::processReceivedFrames);
	//     m_communicator = new Can::Communicator(new Can::FramesStdLogger());
	//     m_communicator->set_task(new Can::FlashTask{});
	//     m_communicator_thread = new CommunicatorThread(
	// 	this, m_communicator, m_communicator_mutex);
	//     connect(m_communicator_thread,
	// 	    &CommunicatorThread::check_frames_to_write, this,
	// 	    &MainWindow::check_frames_to_write);
	//     m_communicator_thread->start();
	// } else {
	//     std::cerr << "Cannot connect device" << std::endl;
	// }
    // }

    setCentralWidget(window);
}

void MainWindow::create_layout(QWidget* root) {

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

    m_logger = new QLogger(this, log_frames, log_messages);

// Options layout
//   devices options layout
    QVBoxLayout* options_layout = new QVBoxLayout(options_group);

    QGroupBox *devices_group = new QGroupBox(tr("Devices"));

    options_layout->addWidget(devices_group);

    QVBoxLayout* devices_group_layout = new QVBoxLayout(devices_group);
    QGroupBox* devices_buttons_group = new QGroupBox();
    QHBoxLayout* devices_buttons_layout = new QHBoxLayout(devices_buttons_group);

    QComboBox* devices_list = new QComboBox(devices_group);
    devices_group_layout->addWidget(devices_list);
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
    devices_list->addItem("Test 1");
    devices_list->addItem("Test 2");

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
