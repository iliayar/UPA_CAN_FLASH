#pragma once

#include "frame.h"
#include "service.h"
#include "task.h"
#include "logger.h"
#include "communicator.h"
#include "qtask.h"

#include <QObject>
#include <chrono>
#include <vector>
#include <string>

class QWorker : public QObject {
    Q_OBJECT
public:
    virtual Can::CommunicatorStatus get_type() = 0;

    std::chrono::milliseconds TIMEOUT{600};

public slots:
    virtual void push_frame(std::shared_ptr<Can::Frame>) = 0;

signals:
    void fetch_frame(std::shared_ptr<Can::Frame>);
    void worker_done();
    void worker_error();
    

protected:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_last_update =
	std::chrono::high_resolution_clock::now();

    bool check_timeout_imp() {
	if (std::chrono::duration_cast<std::chrono::milliseconds>(
		std::chrono::high_resolution_clock::now() - m_last_update) >
	    TIMEOUT) {
	    return false;
	}
	return true;
    }

    void update_imp() {
	m_last_update = std::chrono::high_resolution_clock::now();
    }
};

class QReceiver : public QWorker {
    Q_OBJECT
public:

    Can::CommunicatorStatus get_type() { return Can::CommunicatorStatus::Receive; }
    Can::ServiceResponse* get_response();

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
    void init(Can::ServiceRequest*);
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
    QCommunicator() : QCommunicator(new Can::NoLogger()) {}

    QCommunicator(Can::Logger* logger)
        : m_worker(nullptr),
          m_task(nullptr),
          m_logger(logger),
          m_worker_thread()
          // m_task_thread()
        {
            std::cout << "QCommunicator created" << std::endl;
            m_worker_thread.start();
        }

    void set_task(QTask*);

public slots:
    void push_frame(std::shared_ptr<Can::Frame>);
    void request(Can::ServiceRequest*);
    void fetch_frame_worker(std::shared_ptr<Can::Frame>);

    void worker_done();
    void worker_error();

signals:
    void fetch_frame(std::shared_ptr<Can::Frame>);
    void push_frame_worker(std::shared_ptr<Can::Frame>);
    void response(Can::ServiceResponse*);
    void operate_transmitter(Can::ServiceRequest*);
    void operate_receiver(std::shared_ptr<Can::Frame>);

private:
    void update_task();

    QWorker* m_worker;
    QTask* m_task;
    Can::Logger* m_logger;

    QThread m_worker_thread;
    // QThread m_task_thread;
};
