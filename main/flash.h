#pragma once

#include <string>

#include "task.h"
#include "logger.h"
#include "qtask.h"

class FlashTask : public QTask {
public:
    FlashTask(std::string file, QLogger* logger)
        : QTask(logger), m_file(file) {}
    void task();
    void task_main();
private:
    std::string m_file;
};
