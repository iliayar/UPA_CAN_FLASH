#pragma once

#include "../qtask.h"

#include <QDialog>
#include <QGroupBox>
#include <unordered_map>

// #include "fields.h"
class Field;

class ConfigurationTask : public QTask {

public:

    const std::vector<std::string> GROUPS = {
        "Group1",
        "Group2",
        "Group3",
        "Group4"
    };

    ConfigurationTask(std::shared_ptr<QLogger> logger);

    void task() override;

    friend class Field;

private:

    QDialog* m_main_dialog;
    std::unordered_map<std::string, QGroupBox*> m_groups;
};
