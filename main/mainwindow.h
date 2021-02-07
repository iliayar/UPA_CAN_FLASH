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

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow();
    void processReceivedFrames();
private:
    void communicator_write();
    
    QCanBusDevice *m_device;
    Can::Communicator* m_communicator;
    std::mutex m_communicator_mutex;
    std::thread* m_communicator_thread;
};
