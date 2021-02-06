#include "communicator.h"
#include "frame.h"
#include "task.h"
#include "service.h"

#include <stdexcept>

Can::CommunicatorStatus Can::Communicator::get_status() {
    if(m_worker == nullptr) return Can::CommunicatorStatus::Idle;
    return m_worker->get_type();
}

void Can::Communicator::set_task(Can::Task* task) {
    m_task = task;
    m_worker = new Transmitter(m_task->fetch_request());
}

Can::Frame* Can::Communicator::fetch_frame() {
    if(m_worker == nullptr) throw std::runtime_error("No frames to fetch");
    Can::Frame* frame = m_worker->fetch_frame();
    update_task();
    return frame;
}

void Can::Communicator::update_task() {
    switch(m_worker->get_status()) {
    case Can::WorkerStatus::Done:
        switch(m_worker->get_type()) {
        case Can::CommunicatorStatus::Receive:
            m_task->push_response(static_cast<Can::Receiver*>(m_worker)->get_response());
            delete m_worker;
            break;
        case Can::CommunicatorStatus::Transmit:
            delete m_worker;
            break;
        }
        m_worker = new Transmitter(m_task->fetch_request());
        break;
    case Can::WorkerStatus::Error:
        throw std::runtime_error("Wrong frame received");
        break;
    }
}

void Can::Communicator::push_frame(Can::Frame* frame) {
    if(m_worker == nullptr) {
        m_worker = new Receiver(frame);
    } else {
        m_worker->push_frame(frame);
    }
    update_task();
}

Can::ServiceResponse* Can::frames_to_service(std::vector<Can::Frame*> frames) {
    int len = -1;
    std::vector<uint8_t> payload;
    
    for(Frame* frame : frames) {
        std::vector<uint8_t> data;
        switch(frame->get_type()) {
        case Can::FrameType::SingleFrame:
            len = static_cast<Can::Frame_SingleFrame*>(frame)->get_len();
            data = static_cast<Can::Frame_SingleFrame*>(frame)->get_data();
        case Can::FrameType::FirstFrame:
            data = static_cast<Can::Frame_FirstFrame*>(frame)->get_data();
            len = static_cast<Can::Frame_FirstFrame*>(frame)->get_len();
        case Can::FrameType::ConsecutiveFrame:
            data = static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_data();
        }
        for(int i = 0; i < data.size() && payload.size() < len; ++i) {
            payload.push_back(data[i]);
        }
    }

    return Can::ServiceResponseFactory(payload).get();
}

std::vector<Can::Frame*> Can::service_to_frames(Can::ServiceRequest* request) {
    std::vector<Can::Frame*> frames;
    std::vector<uint8_t> payload = request->dump();

    if(payload.size() > 7) {
        std::vector<uint8_t> frame_data;
        int i = 0;
        for(; i < 6; ++i) {
            frame_data.push_back(payload[i]);
        }
        frames.push_back(new Can::Frame_FirstFrame(payload.size(), frame_data));
        for(int seq = 1; i < payload.size(); ++seq) {
            frame_data.clear();
            for(int j = 0; j < 7; ++i, ++j) {
                if(i >= payload.size()) {
                    frame_data.push_back(0);
                } else {
                    frame_data.push_back(payload[i]);
                }
            }
            frames.push_back(new Can::Frame_ConsecutiveFrame(seq & 0xf, frame_data));
        }
    } else {
        int len = payload.size();
        while(payload.size() < 7) payload.push_back(0);
        frames.push_back(new Can::Frame_SingleFrame(len, payload));
    }

    return frames;
}

Can::Transmitter::Transmitter(Can::ServiceRequest* request) {
}

void Can::Transmitter::push_frame(Can::Frame* frame) {
}

Can::Frame* Can::Transmitter::fetch_frame() {
    return nullptr;
}

Can::WorkerStatus Can::Transmitter::get_status() {
    return Can::WorkerStatus::Error;
}

Can::Receiver::Receiver(Can::Frame* frame) {
    switch (frame->get_type()) {
    case Can::FrameType::SingleFrame: {
        m_frames = {frame};
        m_status = Can::WorkerStatus::Done;
        break;
    }
    case Can::FrameType::FirstFrame: {
        m_frames.push_back(frame);
        m_status = Can::WorkerStatus::Work;
        m_consecutive_len = static_cast<Can::Frame_FirstFrame*>(frame)->get_len();
        m_consecutive_last = 0x0;
        break;
    }
    default:
        // Taking that communicator won't continue with an Error status
        m_status = Can::WorkerStatus::Error;
        break;
    }
}

Can::ServiceResponse* Can::Receiver::get_response() {
    // Taking that communicator only fetch response when status is Ready
    Can::ServiceResponse* response = Can::frames_to_service(m_frames);
    for(Can::Frame* frame : m_frames) {
        delete frame;
    }
    return response;
}

void Can::Receiver::push_frame(Can::Frame* frame) {
    switch (frame->get_type()) {
    case Can::FrameType::ConsecutiveFrame: {
        m_consecutive_last = (m_consecutive_last + 1) & 0xf;
        m_frames.push_back(frame);
        if(m_consecutive_last != static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_seq_num()) {
            m_status = Can::WorkerStatus::Error;
            break;
        }
        if(6 + (m_frames.size() - 1)*7 >= m_consecutive_len) {
            m_status = Can::WorkerStatus::Done;
        }
        break;
    }
    default:
        m_status = Can::WorkerStatus::Error;
        break;
    }
}

Can::Frame* Can::Receiver::fetch_frame() {
    // Only the FlowControl frame has posibility to be fetched
    // :TODO: Various error handling
    return new Frame_FlowControl(Can::FlowStatus::ContinueToSend, 0, 0);
}

Can::WorkerStatus Can::Receiver::get_status() {
    return m_status;
}
