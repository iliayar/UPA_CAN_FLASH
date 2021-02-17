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
#include "util.h"

#define RESPONSE_TIMEOUT 1000
#define IF_NEGATIVE(res) if(res->get_type() == Can::ServiceResponseType::Negative)

class QLoggerWorker : public QObject {
    Q_OBJECT
public:
    QLoggerWorker(QObject*, QTextEdit*, QTextEdit*);

public slots:

    void received_frame(std::shared_ptr<Can::Frame>);
    void transmitted_frame(std::shared_ptr<Can::Frame>);
    void received_service_response(std::shared_ptr<Can::ServiceResponse>);
    void transmitted_service_request(std::shared_ptr<Can::ServiceRequest>);

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
    void received_service_response(std::shared_ptr<Can::ServiceResponse> response) {
        emit signal_received_service_response(response);
    }
    void transmitted_service_request(std::shared_ptr<Can::ServiceRequest> request) {
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

#define DISCONNECT(sig) connect(this, &QLogger::signal_##sig, m_worker, &QLoggerWorker::sig)
    ~QLogger() {
        DISCONNECT(info);
        DISCONNECT(error);
        DISCONNECT(warning);
        DISCONNECT(important);
        DISCONNECT(received_frame);
        DISCONNECT(transmitted_frame);
        DISCONNECT(received_service_response);
        DISCONNECT(transmitted_service_request);
    }
#undef DISCONNECT

signals:
    
    void signal_received_frame(std::shared_ptr<Can::Frame> frame);
    void signal_transmitted_frame(std::shared_ptr<Can::Frame>);
    void signal_received_service_response(std::shared_ptr<Can::ServiceResponse>);
    void signal_transmitted_service_request(std::shared_ptr<Can::ServiceRequest>);

    void signal_error(std::string);
    void signal_info(std::string);
    void signal_warning(std::string);
    void signal_important(std::string);

private:
    QLoggerWorker* m_worker;

};


class QTask : public QThread {
    Q_OBJECT
public:
    QTask(std::shared_ptr<QLogger> logger) : m_logger(logger) {}

    void run() override {
        std::cout << "QTask starting task" << std::endl;
        task();
    }

    virtual void task() = 0;

protected:
    std::shared_ptr<Can::ServiceResponse> call(std::shared_ptr<Can::ServiceRequest>);

public slots:
    void response(std::shared_ptr<Can::ServiceResponse>);

signals:
    void request(std::shared_ptr<Can::ServiceRequest>);
    void response_imp(std::shared_ptr<Can::ServiceResponse>);

private:
    std::shared_ptr<Can::ServiceResponse> m_response;
protected:
    std::shared_ptr<QLogger> m_logger;
};


class QTestTask : public QTask {
    Q_OBJECT
public:
    QTestTask(std::shared_ptr<QLogger> logger) : QTask(logger) {}
    void task();
};
