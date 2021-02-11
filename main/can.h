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

signals:
    
    void signal_received_frame(std::shared_ptr<Can::Frame> frame);
    void signal_transmitted_frame(std::shared_ptr<Can::Frame>);
    void signal_received_service_response(Can::ServiceResponse*);
    void signal_transmitted_service_request(Can::ServiceRequest*);

    void signal_error(std::string);
    void signal_info(std::string);
    void signal_warning(std::string);

private:
    QLoggerWorker* m_worker;

};

class QAsyncTask;

class QAsyncTaskThread : public QThread {
    Q_OBJECT
public:
    QAsyncTaskThread(QAsyncTask* parent, Can::Logger* logger = new Can::NoLogger)
        : m_logger(logger), m_response(nullptr), m_parent(parent) {}

    void run() override {
    }

    virtual void task() = 0;

protected:
    Can::ServiceResponse* call(Can::ServiceRequest* r);

    Can::Logger* m_logger;
signals:
    void request(Can::ServiceRequest*);

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
        : m_completed(false), m_thread(thread) {
        connect(this, &QAsyncTask::response, thread,
                &QAsyncTaskThread::response);
        connect(thread, &QAsyncTaskThread::request, this, &QAsyncTask::request);
        connect(thread, &QAsyncTaskThread::finished, this,
                &QAsyncTask::thread_finished);
        logger->moveToThread(thread);
        thread->start();
    }

    Can::ServiceRequest* fetch_request() {
        while(m_request == nullptr) {
            QSignalSpy spy(m_thread, &QAsyncTaskThread::request);
            spy.wait(RESPONSE_TIMEOUT);
        }

        Can::ServiceRequest* r = m_request;
        m_request = nullptr;
        return r;
    }

    void push_response(Can::ServiceResponse* r) {       
        emit response(r);
    }

    bool is_completed() {
        return m_completed;
    }

signals:
    void response(Can::ServiceResponse*);

public slots:
    void request(Can::ServiceRequest* r) {
        m_request = r;
    }

    void thread_finished() {
        m_completed = true;
    }

private:
    bool m_completed;
    Can::ServiceRequest* m_request;
    QAsyncTaskThread* m_thread;
};
