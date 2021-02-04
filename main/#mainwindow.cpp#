#include "mainwindow.h"

#include <iostream>
#include <QCoreApplication>
#include <QCanBus>
#include <QCanBusFrame>

#ifdef __MINGW32__
#define CAN_PLUGIN "systeccan"
#elif __linux__
#define CAN_PLUGIN "socketcan"
#endif

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
        } else {
            std::cerr << "Cannot connect device" << std::endl;
        }
    }


    setCentralWidget(window);
}

void MainWindow::processReceivedFrames() {
    while(m_device->framesAvailable()) {
        QCanBusFrame frame = m_device->readFrame();
        std::cout << frame.toString().toStdString() << std::endl;
    }
}

MainWindow::~MainWindow()
{}
