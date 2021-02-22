/**
 * @file qlogger.h
 * Implements {@link Can::Logger} interface for displaying in Qt window
 */
#pragma once

#include <QObject>
#include <QTextEdit>
#include <QElapsedTimer>
#include <QThread>
#include <QProgressBar>
#include <QScrollBar>
#include <QDateTime>
#include <memory>

#include "frame.h"
#include "service.h"
#include "util.h"
#include "logger.h"

/**
 * Worker class for {@link QLogger} {@link Logger} implementation
 * Prints message in QTextEdit objects
 */
class QLoggerWorker : public QObject {
    Q_OBJECT
public:
    QLoggerWorker(QObject*, QTextEdit*, QTextEdit*, QProgressBar* bar = nullptr);

public slots:

    void received_frame(std::shared_ptr<Can::Frame>);
    void transmitted_frame(std::shared_ptr<Can::Frame>);
    void received_service_response(std::shared_ptr<Can::ServiceResponse>);
    void transmitted_service_request(std::shared_ptr<Can::ServiceRequest>);

    void error(std::string);
    void info(std::string);
    void warning(std::string);
    void important(std::string);
    void progress(int, bool err);
    
private:
    QString vec_to_qstr(std::vector<uint8_t>);

    QElapsedTimer m_timer;
    
    QTextEdit* m_frame_log;
    QTextEdit* m_message_log;

    QProgressBar* m_progress;

    std::mutex m_mutex;
};

/**
 * Implements {@link Can::Logger} interface on top of {@link QLoggerWorker}
 */
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
        CONNECT(progress);
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
    void progress(int a, bool err = false) {
        emit signal_progress(a, err);
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
        DISCONNECT(progress);
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
    void signal_progress(int, bool);
    
private:
    QLoggerWorker* m_worker;

};
