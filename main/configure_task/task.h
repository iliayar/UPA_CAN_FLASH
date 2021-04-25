#pragma once

#include "../qtask.h"

#include <QLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <QMainWindow>
#include <QCloseEvent>
#include <QSettings>
#include <QEventLoop>
#include <unordered_map>
#include "configuration_window.h"

class ConfigurationTask : public QTask {
Q_OBJECT
public:

    ConfigurationTask(std::shared_ptr<QLogger> logger, bool security, ConfigurationWindow* window);
    virtual ~ConfigurationTask();

    void task() override;

public slots:
    void read(uint16_t);
    void write(uint16_t, std::vector<uint8_t>);
    void read_errors(uint8_t);
    void clear_errors();
    void factory_reset();

signals:
    void read_errors_done(uint8_t, std::vector<std::shared_ptr<Can::DTC>>);
    void read_done(uint16_t, std::vector<uint8_t>);
    void clear_errors_done(bool);

    void windows_closed();

private:
    void diagnostic_session();

    bool m_security;
};
