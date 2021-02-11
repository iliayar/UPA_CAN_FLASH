#pragma once

#include <string>

#include "task.h"
#include "logger.h"
#include "can.h"

class FlashTask : public QAsyncTaskThread {
public:
    FlashTask(std::string file, Can::Logger* logger = new Can::NoLogger())
        : AsyncTask(logger), m_file(file) {}
    void task();
private:
    std::string m_file;
};