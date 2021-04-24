#pragma once

#include "../qtask.h"

#include <QLayout>
#include <QGroupBox>
#include <QTextEdit>
#include <unordered_map>
#include "config.h"

class ConfigurationTask : public QTask {

public:

    ConfigurationTask(std::shared_ptr<QLogger> logger, bool security);

    void task() override;

    friend class Field;

private:
    void read_errors(uint8_t);
    void clear_errors();

    // std::unordered_map<std::string, std::pair<QWidget*, QWidget*>> m_groups;
    QWidget* m_window;
    QTextEdit* m_err_log;
    DataConfig m_config{};

    bool m_security;
};
