#pragma once

#include <QMainWindow>
#include <QSettings>
#include <QTextEdit>
#include <QTabWidget>
#include <QCloseEvent>
#include <QDialog>
#include <QDebug>

#include "logger.h"
#include "configure_task/config.h"
#include "datatypes.h"

class ConfigurationWindow : public QMainWindow {
Q_OBJECT
public:
    explicit ConfigurationWindow(QWidget* parent);
    virtual ~ConfigurationWindow();

signals:
    void closed();
    void read(uint16_t);
    void write(uint16_t, std::vector<uint8_t>);
    void read_errors(uint8_t);
    void clear_errors();
    void factory_reset();

    void read_done_field(uint16_t, std::vector<uint8_t>);

public slots:
    void read_errors_done(uint8_t, std::vector<std::shared_ptr<Can::DTC>>);
    void read_done(uint16_t, std::vector<uint8_t>);
    void clear_errors_done(bool);

    void read_field(uint16_t);
    void write_field(uint16_t, std::vector<uint8_t>);

    void clear_errors_sig();
    void factory_reset_sig();
    void read_errors_sig_failed();
    void read_errors_sig_confirmed();
    
private:
    void create_layout(QTabWidget* tabs);
    void closeEvent(QCloseEvent* e) {
        emit closed();
    }
    
    DataConfig m_config;
    QSettings m_settings;
    QTextEdit* m_err_log;
};
