#pragma once

#include <QCanBus>
#include <QMainWindow>
#include <QThread>
#include <QComboBox>
#include <QLabel>
#include <QSettings>
#include <QSpinBox>
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
    void choose_file();
    void update_devices_list();

    QCanBusDevice* m_device;
    Can::Communicator* m_communicator;
    std::mutex m_communicator_mutex;
    CommunicatorThread* m_communicator_thread;
    Can::Logger* m_logger;
    QLoggerWorker* m_logger_worker;
    std::string m_file;
    QSettings m_settings;

    // UI
    QComboBox* m_device_list;
    QComboBox* m_bitrate_list;
    QComboBox* m_task_list;
    QComboBox* m_plugin_list;

    QSpinBox* m_tester_id_box;
    QSpinBox* m_ecu_id_box;

    QAction* m_file_menu_act;
    
    QLabel* m_filename_label;
    QLabel* m_crc_label;
    QLabel* m_size_label;
    QLabel* m_addr_label;

    int m_tester_id;
    int m_ecu_id;
};
