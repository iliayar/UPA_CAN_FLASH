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

class ConfigurationTask : public QTask {

public:

    ConfigurationTask(std::shared_ptr<QLogger> logger, bool security);
    virtual ~ConfigurationTask();

    void task() override;

    optional<std::vector<uint8_t>> read(uint16_t id);
    void write(uint16_t id, std::vector<uint8_t> vec);
    optional<std::vector<std::shared_ptr<Can::DTC>>> read_errors(uint8_t);
    bool clear_errors();
    bool factory_reset();

private:
    void diagnostic_session();

    bool m_security;
};
