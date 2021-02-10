#pragma once

#include <QCanBus>
#include <QMainWindow>
#include <QThread>
#include <QComboBox>
#include <mutex>

#include "communicator.h"
#include "can.h"

namespace Ui {
class MainWindow;
}

class CommunicatorThread : public QThread {
    Q_OBJECT
public:
    CommunicatorThread(QObject* parent, Can::Communicator* communicator,
		       std::mutex& communicator_mutex)
	: QThread(parent),
	  m_communicator(communicator),
	  m_communicator_mutex(communicator_mutex) {}
    void run() override;
signals:
    void check_frames_to_write(std::shared_ptr<Can::Frame>);

private:
    Can::Communicator* m_communicator;
    std::mutex& m_communicator_mutex;
};

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
    virtual ~MainWindow();
    void processReceivedFrames();
private:
    void create_layout(QWidget*);
    void check_frames_to_write(std::shared_ptr<Can::Frame>);
    void connect_device();
    void start_task();

    QCanBusDevice* m_device;
    Can::Communicator* m_communicator;
    std::mutex m_communicator_mutex;
    CommunicatorThread* m_communicator_thread;
    Can::Logger* m_logger;

    // UI
    QComboBox* m_device_list;
    QComboBox* m_task_list;
};
