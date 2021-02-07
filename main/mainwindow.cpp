#include "mainwindow.h"
#include "communicator.h"
#include "frame.h"
#include "task.h"

#include <iostream>
#include <QCoreApplication>
#include <QCanBus>
#include <QCanBusFrame>
#include <QThread>
#include <QDebug>
#include <vector>

#ifdef __MINGW32__
#define CAN_PLUGIN "systeccan"
#elif __linux__
#define CAN_PLUGIN "socketcan"
#endif

#define DEVICE_ID 0x76E
#define TESTER_ID 0x74E

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent)
    , m_device()
{
    QWidget* window = new QWidget();

    // Listing devices
    QString errorString;
    QList<QCanBusDeviceInfo> devices = QCanBus::instance()->availableDevices(
        QStringLiteral(CAN_PLUGIN), &errorString);
    if(!errorString.isEmpty()) {
        std::cerr << errorString.toStdString() << std::endl;
    }
    else {
        std::cout << "Devices" << std::endl;
        for(auto device : devices) {
            std::cout << "- " << device.name().toStdString() << std::endl;
            std::cout << "  " << device.description().toStdString() << std::endl;
        }
    }

    if(devices.size() == 0) {
        std::cerr << "No devices found" << std::endl;
        exit(1);
    }
    QString device_name = devices[0].name();

    std::cout << "Using device " << device_name.toStdString() << std::endl;
    m_device = QCanBus::instance()->createDevice(
                       QStringLiteral(CAN_PLUGIN), device_name, &errorString);
    if(!m_device) {
        std::cerr << errorString.toStdString() << std::endl;
    } else {
        std::cout << "Connecting device" << std::endl;
        if(m_device->connectDevice()) {
            std::cout << m_device->state() << std::endl;
            connect(m_device, &QCanBusDevice::framesReceived,
                    this, &MainWindow::processReceivedFrames);
            m_communicator = new Can::Communicator();
            m_communicator->set_task(new Can::ReadWriteThreadedTask{});
            m_communicator_thread = new CommunicatorThread(this, m_communicator, m_communicator_mutex);
            connect(m_communicator_thread, &CommunicatorThread::check_frames_to_write, this, &MainWindow::check_frames_to_write);
            m_communicator_thread->start();
        } else {
            std::cerr << "Cannot connect device" << std::endl;
        }
    }

    setCentralWidget(window);
}

void MainWindow::check_frames_to_write(Can::Frame* frame) {
        std::vector<uint8_t> payload = frame->dump();
        QCanBusFrame qframe;
        qframe.setFrameId(TESTER_ID);
        qframe.setPayload(QByteArray(reinterpret_cast<const char*>(payload.data()), payload.size()));
        m_device->writeFrame(qframe);
}

void MainWindow::processReceivedFrames() {
    std::unique_lock<std::mutex> lock(m_communicator_mutex);
    while(m_device->framesAvailable()) {
        QCanBusFrame qframe = m_device->readFrame();
        if(qframe.frameId() == DEVICE_ID) {
            QByteArray payload = qframe.payload();
            Can::Frame* frame = Can::FrameFactory(std::vector<uint8_t>(payload.begin(), payload.end())).get();
            m_communicator->push_frame(frame);
        }
    }
}

MainWindow::~MainWindow()
{}

void CommunicatorThread::run() {
    while(true) {
	{
            std::unique_lock<std::mutex> lock(m_communicator_mutex);
	    try {
		    Can::Frame* frame = m_communicator->fetch_frame();
		    emit check_frames_to_write(frame);
	    } catch(Can::NothingToFetch e) {

	    }
        }
    }	
}
