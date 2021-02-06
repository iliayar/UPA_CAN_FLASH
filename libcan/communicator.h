#pragma once

#include "frame.h"
#include "task.h"
#include "service.h"

#include <vector>

namespace Can {

    enum class CommunicatorStatus {
        Idle,
        Receive,
        Transmit
    };

    enum WorkerStatus {
        Work,
        Error,
        Done
    };

    std::vector<Frame*> service_to_frames(ServiceRequest*);
    ServiceResponse* frames_to_service(std::vector<Frame*>);
    
    class Worker {
    public:
        virtual CommunicatorStatus get_type() = 0;
        virtual WorkerStatus get_status() = 0;
        virtual Frame* fetch_frame() = 0;
        virtual void push_frame(Frame*) = 0;
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
    };

    class Communicator {
    public:
        Communicator()
            : m_worker(nullptr)
            {}
        
        CommunicatorStatus get_status();

        void set_task(Task*);

        Frame* fetch_frame();
        void push_frame(Frame*);
    private:
        void update_task();
        
        Worker* m_worker;
        Task* m_task;
    };
    
}
