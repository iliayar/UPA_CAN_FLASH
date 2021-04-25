#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QTextEdit>
#include <QTabWidget>
#include <QCloseEvent>
#include <QDialog>

#include "logger.h"
#include "config.h"
#include "task.h"

class ConfigurationWindow : public QDialog {
Q_OBJECT
public:
    explicit ConfigurationWindow(QWidget* parent, ConfigurationTask* task);
    virtual ~ConfigurationWindow();

signals:
    void closed();
private:
    void create_layout(QTabWidget* tabs);
    void read_errors(uint8_t);
    void clear_errors();
    void factory_reset();
    void closeEvent(QCloseEvent e) {
        emit closed();
    }
    
    DataConfig m_config{};
    QSettings m_settings;
    QTextEdit* m_err_log;
    ConfigurationTask* m_task;
};
