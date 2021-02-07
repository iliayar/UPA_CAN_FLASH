#pragma once

#include <chrono>
#include <vector>

#include "frame.h"
#include "service.h"
#include "task.h"

namespace Can {

struct NothingToFetch : public std::exception {
    const char* what() const throw() { return "No frames to fetch"; }
};

enum class CommunicatorStatus { Idle, Receive, Transmit };

enum WorkerStatus { Done, Work, Error };

std::vector<Frame*> service_to_frames(ServiceRequest*);
ServiceResponse* frames_to_service(std::vector<Frame*>);

class Worker {
public:
    virtual CommunicatorStatus get_type() = 0;
    virtual WorkerStatus get_status() = 0;
    virtual Frame* fetch_frame() = 0;
    virtual void push_frame(Frame*) = 0;

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

class Receiver : public Worker {
public:
    Receiver(Frame*);

    CommunicatorStatus get_type() { return CommunicatorStatus::Receive; }
    WorkerStatus get_status();
    Frame* fetch_frame();
    void push_frame(Frame*);
    ServiceResponse* get_response();

private:
    std::vector<Frame*> m_frames;
    WorkerStatus m_status;

    int m_consecutive_len;
    int m_consecutive_last;
};

class Transmitter : public Worker {
public:
    Transmitter(ServiceRequest*);

    CommunicatorStatus get_type() { return CommunicatorStatus::Transmit; }
    WorkerStatus get_status();
    Frame* fetch_frame();
    void push_frame(Frame*);

private:
    std::vector<Frame*> m_frames;
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
    Communicator() : m_worker(nullptr), m_task(nullptr) {}

    CommunicatorStatus get_status();

    void set_task(Task*);

    Frame* fetch_frame();
    void push_frame(Frame*);

private:
    void update_task();

    Worker* m_worker;
    Task* m_task;
};

}  // namespace Can
