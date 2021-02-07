#pragma once

#include "communicator.h"

#include <QMainWindow>
#include <QCanBus>
#include <QThread>
#include <mutex>

namespace Ui
{
    class MainWindow;
}

class CommunicatorThread : public QThread {
    Q_OBJECT

public:
    CommunicatorThread(QObject* parent, QCanBusDevice* device, Can::Communicator* communicator, std::mutex& mutex)
        : QThread(parent)
        , m_device(device)
        , m_communicator(communicator)
        , m_communicator_mutex(mutex) 
        {}
    
    void run() override;
private:
    std::mutex& m_communicator_mutex;
    Can::Communicator* m_communicator;
    QCanBusDevice* m_device;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow();
    void processReceivedFrames();
private:
    QCanBusDevice *m_device;
    Can::Communicator* m_communicator;
    std::mutex m_communicator_mutex;
    CommunicatorThread* m_communicator_thread;
};
