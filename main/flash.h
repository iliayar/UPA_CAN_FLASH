#pragma once

#include <string>

#include "task.h"
#include "logger.h"

class FlashTask : public Can::AsyncTask {
public:
    FlashTask(std::string file, Can::Logger* logger = new Can::NoLogger())
        : AsyncTask(logger), m_file(file) {}
    void task();
private:
    std::string m_file;
};