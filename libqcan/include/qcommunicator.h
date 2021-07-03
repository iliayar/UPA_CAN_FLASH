/**
 * @file qcommunicator.h
 * Defines reworked with Qt signals
 * Communicator classes from libcan
 */
#pragma once

#include <QObject>
#include <QTimer>
#include <chrono>
#include <string>
#include <vector>

#include "communicator.h"
#include "config.h"
#include "frame.h"
#include "logger.h"
#include "qtask.h"
#include "service.h"
#include "task.h"

class QCommunicator;

/**
 * @constant timeout in ms to wait next frame for
 */
#ifndef FRAME_TIMEOUT
#define FRAME_TIMEOUT 1000
#endif

enum class WorkerError { Timeout, Other };

/**
 * Abstract worker class usining in QCommunictor
 * to receive/transmit frames.
 * Implements timeout logic
 */
class QWorker : public QObject {
    Q_OBJECT
public:
    QWorker(QCommunicator const*);

    virtual optional<std::shared_ptr<Can::ServiceResponse::ServiceResponse>>
    try_get_response() = 0;

    /**
     * Disconnecting all timer related stuff from worker object
     */
    virtual ~QWorker() {
        disconnect(&m_timer, &QTimer::timeout, this, &QWorker::timeout);
        disconnect(this, &QWorker::start_timer, &m_timer,
                   static_cast<void (QTimer::*)(int)>(&QTimer::start));
        disconnect(this, &QWorker::stop_timer, &m_timer, &QTimer::stop);
    }

    /**
     * @return Enum type of worker
     */
    virtual Can::CommunicatorStatus get_type() = 0;

public slots:
    /**
     * Handle new received frame
     * @param frame receiver from ECU
     */
    virtual void push_frame(std::shared_ptr<Can::Frame::Frame>) = 0;

    /**
     * Using by timer to notify timeout
     */
    void timeout() { emit worker_error(WorkerError::Timeout); }
signals:

    /**
     * @param frame Frame to send to ECU
     * Emits when new frame is ready to send
     */
    void fetch_frame(std::shared_ptr<Can::Frame::Frame>);

    /**
     * signal
     * Emits when worker have sended/recieved all frames
     */
    void worker_done();

    /**
     * @signal
     * Emits when error ocues during trasmitting/receiving request/response
     */
    void worker_error(WorkerError);

    /**
     * @signal
     * Emits on inint. Starts timer
     */
    void start_timer(int);

    /**
     * @signal
     * Emits on destructing. Stops timer
     */
    void stop_timer();

protected:
    /**
     * Updates timer when new frame received/transimtted
     */
    void update_timer() { emit start_timer(FRAME_TIMEOUT); }

    /**
     * Initialize timer. Must be called in the thread worker will work in.
     */
    void init_timer() {
        connect(&m_timer, &QTimer::timeout, this, &QWorker::timeout);
        connect(this, &QWorker::start_timer, &m_timer,
                static_cast<void (QTimer::*)(int)>(&QTimer::start));
        connect(this, &QWorker::stop_timer, &m_timer, &QTimer::stop);
        emit start_timer(FRAME_TIMEOUT);
    }

    QCommunicator const* m_communicator;

private:
    QTimer m_timer;
};

/**
 * Implemetation of {@link QWorker}. Received complete response from ECU
 */
class QReceiver : public QWorker {
    Q_OBJECT
public:
    QReceiver(QCommunicator const*);

    Can::CommunicatorStatus get_type() override {
        return Can::CommunicatorStatus::Receive;
    }

    /**
     * @return ServiceResponse pointer to parsed response or nullptr if
     * received frame do not represent a valid reponse
     */
    optional<std::shared_ptr<Can::ServiceResponse::ServiceResponse>>
    try_get_response() override;

    ~QReceiver();

public slots:
    /**
     * Initialize reciever based on passed frame. Must be called
     * after moving to worker thread
     * @param frame to initlize reciever from
     */
    void init(std::shared_ptr<Can::Frame::Frame>);
    void push_frame(std::shared_ptr<Can::Frame::Frame>) override;

signals:
    void fetch_frame(std::shared_ptr<Can::Frame::Frame>);

private:
    std::vector<std::shared_ptr<Can::Frame::Frame>> m_frames;
    Can::WorkerStatus m_status;

    int m_consecutive_len;
    int m_consecutive_last;
    bool m_was_fc;
};

/**
 * Implemetation of {@link QWorker} that transmits request ot ECU
 */
class QTransmitter : public QWorker {
    Q_OBJECT
public:
    QTransmitter(QCommunicator const*);

    Can::CommunicatorStatus get_type() override {
        return Can::CommunicatorStatus::Transmit;
    }

    optional<std::shared_ptr<Can::ServiceResponse::ServiceResponse>>
    try_get_response() override;

    ~QTransmitter();

public slots:

    /**
     * Initialize reciever based on passed request from task.
     * Must be called after moving to worker thread.
     * @param request to initlize transmitter from
     */
    void init(std::shared_ptr<Can::ServiceRequest::ServiceRequest>);
    void push_frame(std::shared_ptr<Can::Frame::Frame>) override;

signals:
    void fetch_frame(std::shared_ptr<Can::Frame::Frame>);

private:
    std::vector<std::shared_ptr<Can::Frame::Frame>> m_frames;
    Can::WorkerStatus m_status;

    int m_fc_block_size;
    std::chrono::milliseconds m_fc_min_time;
    std::chrono::time_point<std::chrono::high_resolution_clock>
        m_last_frame_time;
    int m_i;
    bool m_wait_fc;
    int m_block_begin;
};

/**
 * Main communication class. Operates with workers and tasks.
 */
class QCommunicator : public QObject {
    Q_OBJECT
public:
    QCommunicator() : QCommunicator(std::make_shared<Can::NoLogger>()) {}

    /**
     * @param logger to to write log to
     */
    QCommunicator(std::shared_ptr<Can::Logger> logger)
        : m_worker(nullptr),
          m_task(nullptr),
          m_logger(logger),
          m_worker_thread() {
        m_worker_thread.start();
    }

    ~QCommunicator();

public slots:

    /**
     * Process received from ECU frame
     * Triggers by main programm
     * @param frame recevied from ECU
     */
    void push_frame(std::shared_ptr<Can::Frame::Frame>);

    /**
     * Process request received from task, sends it to ECU
     * Triggers by {@link QTask} class
     * @param request received from task
     */
    void request(std::shared_ptr<Can::ServiceRequest::ServiceRequest>);

    /**
     * Fetching frame frome worker ans pass it to main programm
     * Triggers by woker.
     * @param frame recevied from worker to send to ECU
     */
    void fetch_frame_worker(std::shared_ptr<Can::Frame::Frame>);

    /**
     * Triggers by worker when it's done
     */
    void worker_done();

    /**
     * Triggers by worker when error ocures
     */
    void worker_error(WorkerError);

    /**
     * Set task to execute
     * Triggers by main programm.
     * @param task implements {@link QTask}
     */
    void set_task(std::shared_ptr<QTask>);

    /**
     * Triggers by worker {@link QWorker::task_done} method
     */
    void task_done();

signals:

    /**
     * Emits when received frame to worker and apss it to main programm
     * @param frame received from worker
     */
    void fetch_frame(std::shared_ptr<Can::Frame::Frame>);

    /**
     * Emits when receive frame from main programm.
     * Then pass it to current worker
     * @param frame received from ECU
     */
    void push_frame_worker(std::shared_ptr<Can::Frame::Frame>);

    /**
     * Emits when worker done and there is not nullptr response parsed by one
     * It's passed to current task
     * @param response fetched from receiver
     * @param wait flag. if not 0 then ignore {@param response} and increase
     * task timeout by wait*1000 ms
     */
    void response(std::shared_ptr<Can::ServiceResponse::ServiceResponse>,
                  int wait = 0);

    /**
     * Initialize transmitter. Calls whe transmitter is already in worker thread
     * @param request to initialize trasmitter from
     */
    void operate_transmitter(
        std::shared_ptr<Can::ServiceRequest::ServiceRequest>);

    /**
     * Initialize receiver. Calls when receier is already in worker thread
     * @param frame to initialize receiver from
     */
    void operate_receiver(std::shared_ptr<Can::Frame::Frame>);

    /**
     * Emits when current task called {@link task_done}.
     * Notify main programm that task done.
     */
    void task_exited();

private:
    std::unique_ptr<QWorker> m_worker;
    std::shared_ptr<QTask> m_task;
    std::shared_ptr<Can::Logger> m_logger;

    QThread m_worker_thread;
};
