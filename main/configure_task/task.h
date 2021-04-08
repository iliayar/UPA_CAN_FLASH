#pragma once

#include "../qtask.h"

#include <QLayout>
#include <QGroupBox>
#include <unordered_map>

class ConfigurationTask : public QTask {

public:

    ConfigurationTask(std::shared_ptr<QLogger> logger);

    void task() override;

    friend class Field;

private:

    std::unordered_map<std::string, std::pair<QWidget*, QWidget*>> m_groups;
    QWidget* m_window;
};
