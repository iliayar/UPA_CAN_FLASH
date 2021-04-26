/**
 * @file qtask.h
 * Defined interface of task passed into {@link QCommunicator}
 */
#pragma once

#include <QElapsedTimer>
#include <QObject>
#include <QSignalSpy>
#include <QThread>
#include <memory>

#include "communicator.h"
#include "config.h"
#include "frame.h"
#include "qlogger.h"
#include "service.h"
#include "service_all.h"
#include "task.h"
#include "util.h"

/**
 * Timeout of receiving complete response
 */
#ifndef RESPONSE_TIMEOUT
#define RESPONSE_TIMEOUT 1000
#endif
#define IF_NEGATIVE(res) \
    if (res->get_type() == Can::ServiceResponse::Type::Negative)

/**
 * Interface of task passed into {@link QCommunicator}
 */
class QTask : public QThread {
    Q_OBJECT
public:
    QTask(std::shared_ptr<QLogger> logger) : m_logger(logger), m_wait(0) {}

    /**
     * Run this task.
     */
    void run() override {
        try {
            task();
        } catch (std::experimental::bad_optional_access& e) {
            m_logger->error(std::string("Task failed: ") + e.what());
        }
        DEBUG(info, "Exiting task");
    }

    /**
     * The function called by {@link QThread::run} method to start
     * task. This is the entry point for non abstact tasks.
     */
    virtual void task() = 0;

protected:
    /**
     * Method to call in task(). Provides convenient way to communicate with ECU
     */
    std::shared_ptr<Can::ServiceResponse::ServiceResponse> call(
        std::shared_ptr<Can::ServiceRequest::ServiceRequest>);

    /**
     * Shorthand to gain the the access using provided mask
     * @param mask appropriate mask
     * @return true if security access was gained, false otherwise
     */
    bool security_access(uint32_t mask);
    
public slots:
    /**
     * Triggers by {@link QCommunicator} when complete response recieved.
     * @param response from ECU
     * @param wait for more inforamtion watch {@link QCommunicator::response}
     */
    void response(std::shared_ptr<Can::ServiceResponse::ServiceResponse>, int wait = 0);

signals:

    /**
     * Emits when new request to ECU ready
     * @param request passed to communicator
     */
    void request(std::shared_ptr<Can::ServiceRequest::ServiceRequest>);
    void response_imp(std::shared_ptr<Can::ServiceResponse::ServiceResponse>);

private:
    std::shared_ptr<Can::ServiceResponse::ServiceResponse> m_response;
    int m_wait;

protected:
    std::shared_ptr<QLogger> m_logger;
};
