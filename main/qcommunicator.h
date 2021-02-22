/**
 * @file qcommunicator.h
 * Defines reworked with Qt signals
 * Communicator classes from libcan
 */
#pragma once

#include "frame.h"
#include "service.h"
#include "task.h"
#include "logger.h"
#include "communicator.h"
#include "qtask.h"
#include "config.h"

#include <QObject>
#include <QTimer>
#include <chrono>
#include <vector>
#include <string>

/**
 * @constant timeout in ms to wait next frame for
 */
#ifndef FRAME_TIMEOUT
#define FRAME_TIMEOUT 1000
#endif

enum class WorkerError {
    Timeout,
    Other
};

/**
 * Abstract worker class usining in QCommunictor 
 * to receive/transmit frames.
 * Implements timeout logic
 */
class QWorker : public QObject {
    Q_OBJECT
public:
    QWorker() { }

    /**
     * Disconnecting all timer related stuff from worker object
     */
    ~QWorker() {
        disconnect(&m_timer, &QTimer::timeout, this, &QWorker::timeout);
        disconnect(this, &QWorker::start_timer, &m_timer, static_cast<void (QTimer::*)(int)>(&QTimer::start));
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
    virtual void push_frame(std::shared_ptr<Can::Frame>) = 0;

    /**
     * Using by timer to notify timeout
     */
    void timeout() {
        emit worker_error(WorkerError::Timeout);
    }
signals:

    /**
     * @param frame Frame to send to ECU
     * Emits when new frame is ready to send
     */
    void fetch_frame(std::shared_ptr<Can::Frame>);

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
    void update_timer() {
        emit start_timer(FRAME_TIMEOUT);
    }

    /**
     * Initialize timer. Must be called in the thread worker will work in.
     */
    void init_timer() {
        connect(&m_timer, &QTimer::timeout, this, &QWorker::timeout);
        connect(this, &QWorker::start_timer, &m_timer, static_cast<void (QTimer::*)(int)>(&QTimer::start));
        connect(this, &QWorker::stop_timer, &m_timer, &QTimer::stop);
        emit start_timer(FRAME_TIMEOUT);
    }

private:
    QTimer m_timer; 
};

/**
 * Implemetation of {@link QWorker}. Received complete response from ECU
 */
class QReceiver : public QWorker {
    Q_OBJECT
public:

    Can::CommunicatorStatus get_type() { return Can::CommunicatorStatus::Receive; }

    /**
     * @return ServiceResponse pointer to parsed response or nullptr if
     * received frame do not represent a valid reponse
     */
    std::shared_ptr<Can::ServiceResponse> get_response();

public slots:
    /**
     * Initialize reciever based on passed frame. Must be called 
     * after moving to worker thread
     * @param frame to initlize reciever from
     */
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

/**
 * Implemetation of {@link QWorker} that transmits request ot ECU
 */
class QTransmitter : public QWorker {
    Q_OBJECT
public:

    Can::CommunicatorStatus get_type() { return Can::CommunicatorStatus::Transmit; }

public slots:

    /**
     * Initialize reciever based on passed request from task. 
     * Must be called after moving to worker thread.
     * @param request to initlize transmitter from
     */
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
          m_worker_thread()
        {
            m_worker_thread.start();
        }

public slots:

    /**
     * Process received from ECU frame
     * Triggers by main programm
     * @param frame recevied from ECU
     */
    void push_frame(std::shared_ptr<Can::Frame>);

    /**
     * Process request received from task, sends it to ECU
     * Triggers by {@link QTask} class
     * @param request received from task
     */
    void request(std::shared_ptr<Can::ServiceRequest>);

    /**
     * Fetching frame frome worker ans pass it to main programm
     * Triggers by woker.
     * @param frame recevied from worker to send to ECU
     */
    void fetch_frame_worker(std::shared_ptr<Can::Frame>);

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
    void fetch_frame(std::shared_ptr<Can::Frame>);

    /**
     * Emits when receive frame from main programm.
     * Then pass it to current worker
     * @param frame received from ECU
     */
    void push_frame_worker(std::shared_ptr<Can::Frame>);

    /**
     * Emits when worker done and there is not nullptr response parsed by one
     * It's passed to current task
     * @param response fetched from receiver
     * @param wait flag. if not 0 then ignore {@param response} and increase task timeout by wait*1000 ms
     */
    void response(std::shared_ptr<Can::ServiceResponse>, int wait = 0);

    /**
     * Initialize transmitter. Calls whe transmitter is already in worker thread
     * @param request to initialize trasmitter from
     */
    void operate_transmitter(std::shared_ptr<Can::ServiceRequest>);


    /**
     * Initialize receiver. Calls when receier is already in worker thread
     * @param frame to initialize receiver from
     */
    void operate_receiver(std::shared_ptr<Can::Frame>);

    /**
     * Emits when current task called {@link task_done}.
     * Notify main programm that task done.
     */
    void task_exited();

private:
    std::shared_ptr<QWorker> m_worker;
    std::shared_ptr<QTask> m_task;
    std::shared_ptr<Can::Logger> m_logger;

    QThread m_worker_thread;
};
