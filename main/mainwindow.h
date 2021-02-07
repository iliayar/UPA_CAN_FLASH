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
    CommunicatorThread(QObject* parent)
        : QThread(parent)
        {}
    void run() override;
signals:
    void check_frames_to_write();
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow();
    void processReceivedFrames();
private:
    void check_frames_to_write();
    
    QCanBusDevice *m_device;
    Can::Communicator* m_communicator;
    std::mutex m_communicator_mutex;
    CommunicatorThread* m_communicator_thread;
};
