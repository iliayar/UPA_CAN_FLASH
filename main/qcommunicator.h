#pragma once

#include "frame.h"
#include "service.h"
#include "task.h"
#include "logger.h"
#include "communicator.h"
#include "qtask.h"

#include <QObject>
#include <QTimer>
#include <chrono>
#include <vector>
#include <string>


#define FRAME_TIMEOUT 1000

enum class WorkerError {
    Timeout,
    Other
};


class QWorker : public QObject {
    Q_OBJECT
public:
    QWorker() { }

    ~QWorker() {
        disconnect(&m_timer, &QTimer::timeout, this, &QWorker::timeout);
        disconnect(this, &QWorker::start_timer, &m_timer, static_cast<void (QTimer::*)(int)>(&QTimer::start));
        disconnect(this, &QWorker::stop_timer, &m_timer, &QTimer::stop);
    }
    
    virtual Can::CommunicatorStatus get_type() = 0;

public slots:
    virtual void push_frame(std::shared_ptr<Can::Frame>) = 0;
    void timeout() {
        emit worker_error(WorkerError::Timeout);
        // emit worker_done();
    }

signals:
    void fetch_frame(std::shared_ptr<Can::Frame>);
    void worker_done();
    void worker_error(WorkerError);
    void start_timer(int);
    void stop_timer();

protected:
    void update_timer() {
        emit start_timer(FRAME_TIMEOUT);
    }
    void init_timer() {
        connect(&m_timer, &QTimer::timeout, this, &QWorker::timeout);
        connect(this, &QWorker::start_timer, &m_timer, static_cast<void (QTimer::*)(int)>(&QTimer::start));
        connect(this, &QWorker::stop_timer, &m_timer, &QTimer::stop);
        emit start_timer(FRAME_TIMEOUT);
    }

private:
    QTimer m_timer; 
};

class QReceiver : public QWorker {
    Q_OBJECT
public:

    Can::CommunicatorStatus get_type() { return Can::CommunicatorStatus::Receive; }
    std::shared_ptr<Can::ServiceResponse> get_response();

public slots:
    void init(std::shared_ptr<Can::Frame>);
    void push_frame(std::shared_ptr<Can::Frame>);

signals:
    void fetch_frame(std::shared_ptr<Can::Frame>);

private:
    std::vector<std::shared_ptr<Can::Frame>> m_frames;
    Can::WorkerStatus m_status;

    int m_consecutive_len;
    int m_consecutive_last;
    bool m_was_fc;
};

class QTransmitter : public QWorker {
    Q_OBJECT
public:

    Can::CommunicatorStatus get_type() { return Can::CommunicatorStatus::Transmit; }

public slots:
    void init(std::shared_ptr<Can::ServiceRequest>);
    void push_frame(std::shared_ptr<Can::Frame>);

signals:
    void fetch_frame(std::shared_ptr<Can::Frame>);

private:
    std::vector<std::shared_ptr<Can::Frame>> m_frames;
    Can::WorkerStatus m_status;

    int m_fc_block_size;
    std::chrono::milliseconds m_fc_min_time;
    std::chrono::time_point<std::chrono::high_resolution_clock>
    m_last_frame_time;
    int m_i;
    bool m_wait_fc;
    int m_block_begin;
};

class QCommunicator : public QObject {
    Q_OBJECT
public:
    QCommunicator() : QCommunicator(std::make_shared<Can::NoLogger>()) {}

    QCommunicator(std::shared_ptr<Can::Logger> logger)
        : m_worker(nullptr),
          m_task(nullptr),
          m_logger(logger),
          m_worker_thread()
          // m_task_thread()
        {
            std::cout << "QCommunicator created" << std::endl;
            m_worker_thread.start();
        }

public slots:
    void push_frame(std::shared_ptr<Can::Frame>);
    void request(std::shared_ptr<Can::ServiceRequest>);
    void fetch_frame_worker(std::shared_ptr<Can::Frame>);

    void worker_done();
    void worker_error(WorkerError);

    void set_task(std::shared_ptr<QTask>);
    void task_done();

signals:
    void fetch_frame(std::shared_ptr<Can::Frame>);
    void push_frame_worker(std::shared_ptr<Can::Frame>);
    void response(std::shared_ptr<Can::ServiceResponse>, bool wait = false);
    void operate_transmitter(std::shared_ptr<Can::ServiceRequest>);
    void operate_receiver(std::shared_ptr<Can::Frame>);
    void task_exited();

private:
    void update_task();

    std::shared_ptr<QWorker> m_worker;
    std::shared_ptr<QTask> m_task;
    std::shared_ptr<Can::Logger> m_logger;

    QThread m_worker_thread;
    // QThread m_task_thread;
};
