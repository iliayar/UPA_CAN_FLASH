#pragma once

#include <QObject>
#include <QTextEdit>
#include <QElapsedTimer>
#include <QThread>
#include <QSignalSpy>
#include <memory>

#include "communicator.h"
#include "frame.h"
#include "service.h"
#include "service_all.h"
#include "task.h"

#define RESPONSE_TIMEOUT 10000

class QLoggerWorker : public QObject {
    Q_OBJECT
public:
    QLoggerWorker(QObject*, QTextEdit*, QTextEdit*);

public slots:

    void received_frame(std::shared_ptr<Can::Frame>);
    void transmitted_frame(std::shared_ptr<Can::Frame>);
    void received_service_response(Can::ServiceResponse*);
    void transmitted_service_request(Can::ServiceRequest*);

    void error(std::string);
    void info(std::string);
    void warning(std::string);
    void important(std::string);
    
private:
    QString vec_to_qstr(std::vector<uint8_t>);

    QElapsedTimer m_timer;
    
    QTextEdit* m_frame_log;
    QTextEdit* m_message_log;

    std::mutex m_mutex;
};

class QLogger : public QObject, public Can::Logger {
    Q_OBJECT
public:
#define CONNECT(sig) connect(this, &QLogger::signal_##sig, worker, &QLoggerWorker::sig)
    QLogger(QLoggerWorker* worker) : m_worker(worker) {
        CONNECT(info);
        CONNECT(error);
        CONNECT(warning);
        CONNECT(important);
        CONNECT(received_frame);
        CONNECT(transmitted_frame);
        CONNECT(received_service_response);
        CONNECT(transmitted_service_request);
    }
#undef CONNECT

    QLoggerWorker* get_worker() { return m_worker; }
    void received_frame(std::shared_ptr<Can::Frame> frame) {
        emit signal_received_frame(frame);
    }
    void transmitted_frame(std::shared_ptr<Can::Frame> frame) {
        emit signal_transmitted_frame(frame);
    }
    void received_service_response(Can::ServiceResponse* response) {
        emit signal_received_service_response(response);
    }
    void transmitted_service_request(Can::ServiceRequest* request) {
        emit signal_transmitted_service_request(request);
    }

    void error(std::string s) {
        emit signal_error(s);
    }
    void info(std::string s) {
        emit signal_info(s);
    }
    void warning(std::string s) {
        emit signal_warning(s);
    }
    void important(std::string s) {
        emit signal_important(s);
    }

signals:
    
    void signal_received_frame(std::shared_ptr<Can::Frame> frame);
    void signal_transmitted_frame(std::shared_ptr<Can::Frame>);
    void signal_received_service_response(Can::ServiceResponse*);
    void signal_transmitted_service_request(Can::ServiceRequest*);

    void signal_error(std::string);
    void signal_info(std::string);
    void signal_warning(std::string);
    void signal_important(std::string);

private:
    QLoggerWorker* m_worker;

};

class QAsyncTask;

class QAsyncTaskThread : public QThread {
    Q_OBJECT
public:
    QAsyncTaskThread()
        : m_logger(new Can::NoLogger()), m_response(nullptr), m_parent(nullptr) {}

    void set_logger(QLogger* logger) {
        m_logger = logger;
    }

    void set_controller(QAsyncTask* parent) {
        m_parent = parent;
    }

    virtual void task() = 0;

protected:

    void run() override {
        std::cout << "thread started" << std::endl;
        m_logger->info("Thread started");
        task();
    }

protected:
    Can::ServiceResponse* call(Can::ServiceRequest* r);

    Can::Logger* m_logger;
signals:
    void request(Can::ServiceRequest*);
    void wait_response();
    void unwait_response();

public slots:
    void response(Can::ServiceResponse* r) {
        m_response = r;
    }

private:
    QAsyncTask* m_parent;
    Can::ServiceResponse* m_response;
};

class QAsyncTask : public QObject, public Can::Task {
Q_OBJECT
public:
    QAsyncTask(QAsyncTaskThread* thread, QLogger* logger)
        : m_completed(false), m_thread(thread), Task(), m_request(nullptr), m_wait_response(false) {
        connect(this, &QAsyncTask::response, thread,
                &QAsyncTaskThread::response);
        connect(thread, &QAsyncTaskThread::request, this, &QAsyncTask::request);
        connect(thread, &QAsyncTaskThread::wait_response, this, &QAsyncTask::wait_response);
        connect(thread, &QAsyncTaskThread::unwait_response, this, &QAsyncTask::unwait_response);
        connect(thread, &QAsyncTaskThread::finished, this,
                &QAsyncTask::thread_finished);
        logger->moveToThread(thread);
        thread->set_logger(logger);
        thread->set_controller(this);
        thread->setParent(this);
        thread->start();
        std::cout << "thread started??" << std::endl;
    }

    Can::ServiceRequest* fetch_request() {
        std::cout << "fetch_request" << std::endl;
        while(m_request == nullptr) {
            std::cout << "Waiting for request..." << std::endl;
            QSignalSpy spy(m_thread, &QAsyncTaskThread::request);
            spy.wait(RESPONSE_TIMEOUT);
            std::cout << "Request or timeout" << std::endl;
        }

        Can::ServiceRequest* r = m_request;
        m_request = nullptr;
        return r;
    }

    void push_response(Can::ServiceResponse* r) {       
        std::cout << "push_response" << std::endl;
        emit response(r);
    }

    bool is_completed() {
        std::cout << "is_completed" << std::endl;
        return m_completed;
    }

signals:
    void response(Can::ServiceResponse*);

public slots:
    void request(Can::ServiceRequest* r) {
        m_request = r;
    }

    void wait_response() {
        m_wait_response = true;
    }
    void unwait_response() {
        m_wait_response = false;
    }

    void thread_finished() {
        m_completed = true;
    }

private:
    bool m_completed;
    bool m_wait_response;
    Can::ServiceRequest* m_request;
    QAsyncTaskThread* m_thread;
};
