/**
 * @file qtask.h
 * Defined interface of task passed into {@link QCommunicator}
 */
#pragma once

#include <QObject>
#include <QElapsedTimer>
#include <QThread>
#include <QSignalSpy>
#include <memory>

#include "communicator.h"
#include "frame.h"
#include "service.h"
#include "service_all.h"
#include "task.h"
#include "util.h"
#include "qlogger.h"

/**
 * Timeout of receiving complete response
 */
#define RESPONSE_TIMEOUT 1000
#define IF_NEGATIVE(res) if(res->get_type() == Can::ServiceResponseType::Negative)

/**
 * Interface of task passed into {@link QCommunicator}
 */
class QTask : public QThread {
    Q_OBJECT
public:
    QTask(std::shared_ptr<QLogger> logger) : m_logger(logger), m_wait(0) {}

    void run() override {
        try {
            task();
        } catch(std::runtime_error e) {
            m_logger->error(std::string("Task failed: ") + e.what());
        }
        DEBUG(info, "Exiting task");
    }

    virtual void task() = 0;

protected:

    /**
     * Method to call in task(). Provides convenient way to communicate with ECU
     */
    std::shared_ptr<Can::ServiceResponse> call(std::shared_ptr<Can::ServiceRequest>);

public slots:
    /**
     * Triggers by {@link QCommunicator} when complete response recieved.
     * @param response from ECU
     * @param wait for more inforamtion watch {@link QCommunicator::response}
     */
    void response(std::shared_ptr<Can::ServiceResponse>, int wait = 0);

signals:

    /**
     * Emits when new request to ECU ready
     * @param request passed to communicator
     */
    void request(std::shared_ptr<Can::ServiceRequest>);
    void response_imp(std::shared_ptr<Can::ServiceResponse>);

private:
    std::shared_ptr<Can::ServiceResponse> m_response;
    int m_wait;
protected:
    std::shared_ptr<QLogger> m_logger;
};


class QTestTask : public QTask {
    Q_OBJECT
public:
    QTestTask(std::shared_ptr<QLogger> logger) : QTask(logger) {}
    void task();
};
