/**
 * @file communicator.h
 * These interfaces not in use beacuse of problems
 * with thread and following race conditions. Actual
 * classes has "Q" prefix and located in main/qcommunicator.h
 * That one uses Qt event loops to manage with async tasks
 */
#pragma once

#include <chrono>
#include <string>
#include <vector>

#include "frame.h"
#include "logger.h"
#include "service.h"
#include "task.h"

namespace Can {

enum class CommunicatorStatus { Idle, Receive, Transmit };

enum WorkerStatus { Done, Work, Error };

/**
 * Converts Service request to frames to send to ECU
 * @param service request to convert
 */
optional<std::vector<std::shared_ptr<Frame::Frame>>> service_to_frames(
    std::shared_ptr<ServiceRequest::ServiceRequest>);

/**
 * Converts received frames to service response
 * @param the frames wich one to convert to response
 * @return null if the frames consequnce is invalid
 */
optional<std::shared_ptr<ServiceResponse::ServiceResponse>> frames_to_service(
    std::vector<std::shared_ptr<Frame::Frame>>);

/**
 * Abstract class for receiving/transmitting complete responses/requests
 */
class Worker {
public:
    /**
     * Type of worker to determine Receiver/Transmitter
     */
    virtual CommunicatorStatus get_type() = 0;

    /**
     * Status of worker.
     */
    virtual WorkerStatus get_status() = 0;

    /**
     * Fetch frame from worker to send to ECU.
     * @return frame
     * @throws NothingToFetch if there is no frames to send to ECU
     */
    virtual optional<std::shared_ptr<Frame::Frame>> fetch_frame() = 0;

    /**
     * Push frames, recevied from ECU to worker
     */
    virtual void push_frame(std::shared_ptr<Frame::Frame>) = 0;

    virtual ~Worker() {
    }

    std::chrono::milliseconds TIMEOUT{600};

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

/**
 * Received complete response, sends FlowControl with zero parameters
 * of delay and block size.
 */
class Receiver : public Worker {
public:
    /**
     * @param frame First frame of response recieved. Pass to determine
     * if there is long or short response
     */
    Receiver(std::shared_ptr<Frame::Frame> frame);

    CommunicatorStatus get_type() { return CommunicatorStatus::Receive; }
    WorkerStatus get_status();
    optional<std::shared_ptr<Frame::Frame>> fetch_frame();
    void push_frame(std::shared_ptr<Frame::Frame>);
    optional<std::shared_ptr<ServiceResponse::ServiceResponse>> get_response();

private:
    std::vector<std::shared_ptr<Frame::Frame>> m_frames;
    WorkerStatus m_status;

    int m_consecutive_len;
    int m_consecutive_last;
    bool m_was_fc;
};

class Transmitter : public Worker {
public:
    /**
     * @param Request to send to ECU.
     * @constructor
     */
    Transmitter(std::shared_ptr<ServiceRequest::ServiceRequest>);

    CommunicatorStatus get_type() { return CommunicatorStatus::Transmit; }
    WorkerStatus get_status();
    optional<std::shared_ptr<Frame::Frame>> fetch_frame();
    void push_frame(std::shared_ptr<Frame::Frame>);

private:
    std::vector<std::shared_ptr<Frame::Frame>> m_frames;
    WorkerStatus m_status;

    int m_fc_block_size;
    std::chrono::milliseconds m_fc_min_time;
    std::chrono::time_point<std::chrono::high_resolution_clock>
        m_last_frame_time;
    int m_i;
    bool m_wait_fc;
    int m_block_begin;
};

class Communicator {
public:
    Communicator() : Communicator(new NoLogger()) {}

    /**
     * @param Logger class.
     */
    Communicator(Logger* logger)
        : m_worker(nullptr), m_task(nullptr), m_logger(logger) {}

    /**
     * Get status of communicator
     * @return mostly depends on current worker.
     * If there is not one, returns Idle status
     */
    CommunicatorStatus get_status();

    /**
     * Set task to fetch requests from.
     * It's completely broken as everything in this Communicator
     */
    void set_task(std::shared_ptr<Task> task);
    void reset_task();

    /**
     * Fetch frame from current worker.
     * @throws NothingToFetch if there is no frame.
     */
    optional<std::shared_ptr<Frame::Frame>> fetch_frame();

    /**
     * Push frame to current worker
     * If there is not one, it will be created using
     * the initial frame passed
     * @param frame to push to existing worker or create
     * a new one using this frame as first if response
     */
    void push_frame(std::shared_ptr<Frame::Frame>);

private:
    void update_task();

    optional<std::shared_ptr<Worker>> m_worker;
    optional<std::shared_ptr<Task>> m_task;
    Logger* m_logger;
};

}  // namespace Can
