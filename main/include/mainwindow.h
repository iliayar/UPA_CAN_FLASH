#pragma once

#include <QCanBus>
#include <QComboBox>
#include <QLabel>
#include <QMainWindow>
#include <QWindow>
#include <QProgressBar>
#include <QPushButton>
#include <QSettings>
#include <QSpinBox>
#include <QThread>
#include <QLineEdit>
#include <QCheckBox>
#include <mutex>

#include "communicator.h"
#include "qcommunicator.h"
#include "qtask.h"

namespace Ui {
class MainWindow;
}  // namespace Ui

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget* parent = nullptr);
//     virtual ~MainWindow();
    void processReceivedFrames();
signals:
    void frame_received(std::shared_ptr<Can::Frame::Frame>);
    void set_task(std::shared_ptr<QTask>);

public slots:
    void choose_file();
    void start_task(QString);
    void abort_task();
    void connect_device();
    void check_frames_to_write(std::shared_ptr<Can::Frame::Frame>);
    void disconnect_device();
    void task_done();
    void closeEvent(QCloseEvent *event);

private:
    void create_layout(QWidget*);
    void update_device_list(const QString& str);
    void device_state_changes(QCanBusDevice::CanBusDeviceState state);
    void device_error(QCanBusDevice::CanBusError error);

    QCanBusDevice* m_device;
    QCommunicator* m_communicator;
    std::mutex m_communicator_mutex;
    QThread m_communicator_thread;
    QLogger* m_logger;
    QLoggerWorker* m_logger_worker;
    std::string m_file;
    QSettings m_settings;

    // UI
    QTextEdit* m_log_frames;
    QTextEdit* m_log_messages;

    std::vector<QPushButton*> m_start_task_buttons;
    QPushButton* m_connect_device_button;
    QPushButton* m_disconnect_device_button;

    QComboBox* m_device_list;
    QComboBox* m_bitrate_list;
    QComboBox* m_task_list;
    QComboBox* m_plugin_list;

    QProgressBar* m_progress_bar;

    QSpinBox* m_tester_id_box;
    QSpinBox* m_ecu_id_box;

    QAction* m_file_menu_act;
    QAction* m_settings_menu_act;

    QLabel* m_filename_label;
    QLabel* m_crc_label;
    QLabel* m_size_label;
    QLabel* m_addr_label;

    QCheckBox* m_config_security_checkbox;

    int m_tester_id;
    int m_ecu_id;

    QWidget* m_settings_window;

    QLineEdit* m_mask02;
    QLineEdit* m_mask03;
};
