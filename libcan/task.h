#pragma once

#include <future>
#include <iostream>
#include <mutex>

#include "service.h"

namespace Can {

class Task {
public:
    virtual ServiceRequest* fetch_request() = 0;
    virtual void push_response(ServiceResponse*) = 0;
    virtual bool is_completed() = 0;
};

class ReadWriteTask : public Task {
public:
    ServiceRequest* fetch_request();
    void push_response(ServiceResponse* response);

    bool is_completed();

private:
    int m_step = 0;
};

class AsyncTask : public Task {
public:
    AsyncTask()
	: m_async(&AsyncTask::task_imp, this),
	  m_completed(false),
	  m_request(nullptr),
	  m_response(nullptr) {}

    ServiceRequest* fetch_request() {
	std::this_thread::sleep_for(
	    static_cast<std::chrono::milliseconds>(DELAY));
	while (true) {
	    {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_request != nullptr) {
		    ServiceRequest* request = m_request;
		    m_request = nullptr;
		    return request;
		}
		if (m_wait_response) return nullptr;
	    }
	    if (is_completed()) return nullptr;
	}
    }

    void push_response(ServiceResponse* response) {
	std::this_thread::sleep_for(
	    static_cast<std::chrono::milliseconds>(DELAY));
	while (true) {
	    {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_response == nullptr) {
		    m_response = response;
		    return;
		}
		if (!m_wait_response) return;
	    }
	    if (is_completed()) return;
	}
    }

    // :FIXME: Wanna some race conditions ?!?!?
    bool is_completed() {
	std::this_thread::sleep_for(
	    static_cast<std::chrono::milliseconds>(DELAY));
	return m_completed;
    }

    virtual void task() = 0;

protected:
    ServiceResponse* call(ServiceRequest* request) { return call_imp(request); }

private:
    void task_imp() {
	task();
	m_completed = true;
    }

    ServiceResponse* call_imp(ServiceRequest* request) {
	while (true) {
	    {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_request == nullptr) {
		    m_request = request;
		    m_wait_response = true;
		    break;
		}
	    }
	}
	while (true) {
	    {
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_response != nullptr) {
		    ServiceResponse* response = m_response;
		    m_response = nullptr;
		    m_wait_response = false;
		    return response;
		}
	    }
	}
    }

    ServiceRequest* m_request;
    ServiceResponse* m_response;
    bool m_completed;
    bool m_wait_response;
    std::thread m_async;
    std::mutex m_mutex;

    static constexpr std::chrono::milliseconds DELAY =
	static_cast<std::chrono::milliseconds>(2);
};

class ReadWriteThreadedTask : public AsyncTask {
public:
    void task();
};
}  // namespace Can
