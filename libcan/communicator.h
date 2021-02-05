#pragma once

#include "frame.h"

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
    
    class Worker {
    public:
        virtual CommunicatorStatus get_type() = 0;
        virtual WorkerStatus get_status() = 0;
        virtual Frame* fetch_frame() = 0;
        virtual void push_frame(Frame*) = 0;
    };
    
    class Receiver : public Worker {
    public:
        CommunicatorStatus get_type() { return CommunicatorStatus::Receive; }
        WorkerStatus get_status();
        Frame* fetch_frame();
        void push_frame(Frame*);
    };
    
    class Transmitter : public Worker {
    public:
        CommunicatorStatus get_type() { return CommunicatorStatus::Transmit; }
        WorkerStatus get_status();
        Frame* fetch_frame();
        void push_frame(Frame*);
    };
    
    class Communicator {
    public:
    private:
    };
    
}
