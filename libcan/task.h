/**
 * @file task.h
 * Defines Task interface to pass in communicator class
 * It's not in use too. Use qtask.h from main.
 * It's full of race conditions and thread unsafe operations
 */
#pragma once

#include <future>
#include <iostream>
#include <mutex>

#include "logger.h"
#include "service.h"

#define LOG(level, text) \
    DEBUG(level, text);  \
    m_logger->level(text);

namespace Can {

/**
 * @interface
 * Task actually works only with request
 * It's convient to write algorithm to e.g. flash ECU
 * do not event thinking of lower level, as frames and etc.
 */
class Task {
public:
    /**
     * @return service request to send to ECU
     */
    virtual optional<std::shared_ptr<ServiceRequest::ServiceRequest>> fetch_request() = 0;

    /**
     * @param service response from ECU to process in task
     */
    virtual void push_response(std::shared_ptr<ServiceResponse::ServiceResponse>) = 0;

    /**
     * @return true if task ended
     */
    virtual bool is_completed() = 0;

    virtual ~Task() {
    }
};

/**
 * Example of implemting task interface with switched
 * by steps
 */
class ReadWriteTask : public Task {
public:
    optional<std::shared_ptr<ServiceRequest::ServiceRequest>> fetch_request();
    void push_response(std::shared_ptr<ServiceResponse::ServiceResponse> response);

    bool is_completed();

private:
    int m_step = 0;
};

/**
 * Abstract implemention of Task interface
 * Provide protected call method to call in overriden
 * task methos for convenient send request and receiving
 * response in one line
 */
class AsyncTask : public Task {
public:
    AsyncTask(Logger* logger = new NoLogger())
        : m_completed(false),
          m_request(nullptr),
          m_logger(logger),
          m_thread(&AsyncTask::task_imp, this),
          m_response(nullptr) {}

    optional<std::shared_ptr<ServiceRequest::ServiceRequest>> fetch_request() {
        std::this_thread::sleep_for(
            static_cast<std::chrono::milliseconds>(DELAY));
        DEBUG(info, "task");
        while (true) {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                if (m_request != nullptr) {
                    std::shared_ptr<ServiceRequest::ServiceRequest> request = m_request;
                    DEBUG(info, "task fetched request");
                    m_request = nullptr;
                    return request;
                }
                if (m_wait_response) return {};
            }
            if (is_completed()) return {};
        }
    }

    void push_response(std::shared_ptr<ServiceResponse::ServiceResponse> response) {
        std::this_thread::sleep_for(
            static_cast<std::chrono::milliseconds>(DELAY));
        DEBUG(info, "task");
        while (true) {
            {
                std::unique_lock<std::mutex> lock(m_mutex);
                if (m_response == nullptr) {
                    m_response = response;
                    DEBUG(info, "task pushing response");
                    return;
                }
                if (!m_wait_response) return;
            }
            if (is_completed()) return;
        }
    }

    // :FIXME: Wanna some race conditions ?!?!?
    bool is_completed() {
        std::this_thread::sleep_for(
            static_cast<std::chrono::milliseconds>(DELAY));
        return m_completed;
    }

    virtual void task() = 0;

protected:
    std::shared_ptr<ServiceResponse::ServiceResponse> call(
        std::shared_ptr<ServiceRequest::ServiceRequest> request) {
        return call_imp(request);
    }

    Logger* m_logger;

    std::mutex m_mutex;

private:
    void task_imp() {
        task();
        m_completed = true;
    }

    std::shared_ptr<ServiceResponse::ServiceResponse> call_imp(std::shared_ptr<ServiceRequest::ServiceRequest>);

    std::shared_ptr<ServiceRequest::ServiceRequest> m_request;
    std::shared_ptr<ServiceResponse::ServiceResponse> m_response;
    bool m_completed;
    bool m_wait_response;
    std::thread m_thread;

    static constexpr std::chrono::milliseconds DELAY =
        static_cast<std::chrono::milliseconds>(2);
};

/**
 * Example implementation of abstract AsyncTask
 */
class ReadWriteThreadedTask : public AsyncTask {
public:
    ReadWriteThreadedTask(Logger* logger = new NoLogger())
        : AsyncTask(logger) {}
    void task();
};
}  // namespace Can
