/**
 * @file flash.h
 * Defines task to flash ECU main programm
 */
#pragma once

#include <string>

#include "logger.h"
#include "qtask.h"

/**
 * Implement QTask interface to
 * flash ECU main programm
 */
class FlashTask : public QTask {
public:
    /**
     * @param file Main programm file in Intel HEX format
     * @param logger
     */
    FlashTask(std::string file, std::shared_ptr<QLogger> logger);

    /**
     * Implementation of abstrct method task
     * Main task algorithm
     */
    void task();

protected:
    void task_main();

private:
    std::string m_file;
};
