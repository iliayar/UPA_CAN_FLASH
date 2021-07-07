/**
 * @file qlogger.h
 * Implements {@link Can::Logger} interface for displaying in Qt window
 */
#pragma once

#include <QDateTime>
#include <QElapsedTimer>
#include <QObject>
#include <QProgressBar>
#include <QScrollBar>
#include <QTextEdit>
#include <QThread>
#include <memory>

#include "frame.h"
#include "logger.h"
#include "service.h"
#include "util.h"

/**
 * Worker class for {@link QLogger} {@link Logger} implementation
 * Prints message in QTextEdit objects
 */
class QLoggerWorker : public QObject {
    Q_OBJECT
public:
    QLoggerWorker(QObject*, QTextEdit*, QTextEdit*,
                  QProgressBar* bar = nullptr);

public slots:

    virtual void received_frame(std::shared_ptr<Can::Frame::Frame>);
    virtual void transmitted_frame(std::shared_ptr<Can::Frame::Frame>);
    virtual void received_service_response(std::shared_ptr<Can::ServiceResponse::ServiceResponse>);
    virtual void transmitted_service_request(std::shared_ptr<Can::ServiceRequest::ServiceRequest>);

    virtual void error(std::string const&);
    virtual void info(std::string const&);
    virtual void warning(std::string const&);
    virtual void important(std::string const&);
    virtual void progress(int, bool err);

    void success(std::string const&);

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
#define CONNECT(sig) \
    connect(this, &QLogger::signal_##sig, worker, &QLoggerWorker::sig)
    QLogger(QLoggerWorker* worker) : m_worker(worker) {
        CONNECT(info);
        CONNECT(error);
        CONNECT(warning);
        CONNECT(important);
        CONNECT(success);
        CONNECT(received_frame);
        CONNECT(transmitted_frame);
        CONNECT(received_service_response);
        CONNECT(transmitted_service_request);
        CONNECT(progress);
    }
#undef CONNECT

    QLoggerWorker* get_worker();
    void received_frame(std::shared_ptr<Can::Frame::Frame> frame) override;
    void transmitted_frame(std::shared_ptr<Can::Frame::Frame> frame) override;
    void received_service_response(
        std::shared_ptr<Can::ServiceResponse::ServiceResponse> response) override;
    void transmitted_service_request(
        std::shared_ptr<Can::ServiceRequest::ServiceRequest> request) override;

    void error(std::string const& s) override;
    void info(std::string const& s) override;
    void warning(std::string const& s) override;
    void important(std::string const& s) override;
    void progress(int a, bool err = false);

    void success(std::string const&);

#define DISCONNECT(sig) \
    connect(this, &QLogger::signal_##sig, m_worker, &QLoggerWorker::sig)
    ~QLogger() {
        DISCONNECT(info);
        DISCONNECT(error);
        DISCONNECT(warning);
        DISCONNECT(important);
        DISCONNECT(success);
        DISCONNECT(received_frame);
        DISCONNECT(transmitted_frame);
        DISCONNECT(received_service_response);
        DISCONNECT(transmitted_service_request);
        DISCONNECT(progress);
    }
#undef DISCONNECT

signals:

    void signal_received_frame(std::shared_ptr<Can::Frame::Frame> frame);
    void signal_transmitted_frame(std::shared_ptr<Can::Frame::Frame>);
    void signal_received_service_response(
        std::shared_ptr<Can::ServiceResponse::ServiceResponse>);
    void signal_transmitted_service_request(
        std::shared_ptr<Can::ServiceRequest::ServiceRequest>);

    void signal_error(std::string const&);
    void signal_info(std::string const&);
    void signal_warning(std::string const&);
    void signal_important(std::string const&);
    void signal_progress(int, bool);

    void signal_success(std::string const&);

private:
    QLoggerWorker* m_worker;
};
