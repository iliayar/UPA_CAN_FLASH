#include "communicator.h"

#include <memory>
#include <stdexcept>

#include "frame_all.h"
#include "service.h"
#include "task.h"

Can::CommunicatorStatus Can::Communicator::get_status() {
    update_task();
    if (!m_worker) return Can::CommunicatorStatus::Idle;
    return m_worker.value()->get_type();
}

void Can::Communicator::set_task(std::shared_ptr<Can::Task> task) {
    auto request = task->fetch_request();
    if(!request) {
        DEBUG(info, "Invalid task passed to communicator");
        m_task = {};
    }
    m_task = task;
    m_worker = std::make_shared<Transmitter>(request.value());
    DEBUG(info, "Transmitter created")
}

void Can::Communicator::reset_task() {
    m_task = nullptr;
}

optional<std::shared_ptr<Can::Frame::Frame>> Can::Communicator::fetch_frame() {
    if (!m_worker) return {};
    auto maybe_frame = m_worker.value()->fetch_frame();
    if (!maybe_frame) {
        return {};
    }
    auto frame = maybe_frame.value();
    DEBUG(info, "frame fetched from worker");
    if (frame == nullptr) return {};
    m_logger->transmitted_frame(frame);
    update_task();
    return frame;
}

void Can::Communicator::push_frame(std::shared_ptr<Can::Frame::Frame> frame) {
    if (!m_worker) {
        m_worker = std::make_shared<Receiver>(frame);
        DEBUG(info, "receiver created");
    } else {
        m_worker.value()->push_frame(frame);
    }
    DEBUG(info, "frame pushed");
    m_logger->received_frame(std::move(frame));
    update_task();
}

void Can::Communicator::update_task() {
    if (!m_task || !m_worker) {
        DEBUG(warning, "No worker or task setted in communicator");
        return;
    }
    auto task = m_task.value();
    auto worker = m_worker.value();
    switch (worker->get_status()) {
        case Can::WorkerStatus::Done: {
            DEBUG(info, "worker done");
            switch (worker->get_type()) {
                case Can::CommunicatorStatus::Receive: {
                    auto maybe_response =
                        std::static_pointer_cast<Can::Receiver>(worker)->get_response();
                    if (!maybe_response) {
                        m_worker = {};
                        DEBUG(warning, "Communicator receive None response from worker");
                        return;
                    }
                    auto response = maybe_response.value();
                    m_logger->received_service_response(response);
                    DEBUG(info, "pushing response to task");
                    task->push_response(response);
                    DEBUG(info, "response pushed");
                    break;
                }
                case Can::CommunicatorStatus::Transmit:
                    m_worker = {};
                    break;
                case Can::CommunicatorStatus::Idle:
                    return;
            }
            if (task->is_completed()) {
                m_worker = {};
                m_logger->info("Task Done!");
                return;
            }
            DEBUG(info, "fetching request");
            auto request = task->fetch_request();
            DEBUG(info, "request fetched");
            if (request) {
                m_worker = std::make_shared<Transmitter>(request.value());
                m_logger->transmitted_service_request(request.value());
            } else {
                m_worker = {};
            }
            break;
        }
        case Can::WorkerStatus::Error:
            break;
    }
}

optional<std::shared_ptr<Can::ServiceResponse::ServiceResponse>> Can::frames_to_service(
    std::vector<std::shared_ptr<Can::Frame::Frame>> frames) {
    int len = -1;
    std::vector<uint8_t> payload;

    for (std::shared_ptr<Frame::Frame> frame : frames) {
        std::vector<uint8_t> data;
        if(frame == nullptr) return {};
        switch (frame->get_type()) {
            case Can::Frame::Type::SingleFrame:
                len = std::static_pointer_cast<Can::Frame::SingleFrame>(frame)
                          ->get_len();
                data = std::static_pointer_cast<Can::Frame::SingleFrame>(frame)
                           ->get_data();
                break;
            case Can::Frame::Type::FirstFrame:
                data = std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                           ->get_data();
                len =
                    std::static_pointer_cast<Can::Frame::FirstFrame>(frame)->get_len();
                break;
            case Can::Frame::Type::ConsecutiveFrame:
                data = std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                           ->get_data();
                break;
            default:
                continue;
        }
        for (int i = 0; i < data.size() && payload.size() < len; ++i) {
            payload.push_back(data[i]);
        }
    }

    return Can::ServiceResponse::Factory(payload).get();
}

optional<std::vector<std::shared_ptr<Can::Frame::Frame>>> Can::service_to_frames(
    std::shared_ptr<Can::ServiceRequest::ServiceRequest> request) {
    std::vector<std::shared_ptr<Can::Frame::Frame>> frames;
    auto maybe_payload = request->dump();
    if(!maybe_payload) {
        return {};
    }
    auto payload = maybe_payload.value();

    if (payload.size() > 7) {
        std::vector<uint8_t> frame_data;
        int i = 0;
        for (; i < 6; ++i) {
            frame_data.push_back(payload[i]);
        }
        auto frame = Frame::FirstFrame::build()
            ->len(payload.size())
            ->data(frame_data)
            ->build();
        if(!frame) {
            return {};
        }
        frames.push_back(frame.value());
        for (int seq = 1; i < payload.size(); ++seq) {
            frame_data.clear();
            for (int j = 0; j < 7; ++i, ++j) {
                if (i >= payload.size()) {
                    frame_data.push_back(0);
                } else {
                    frame_data.push_back(payload[i]);
                }
            }
            auto frame = Frame::ConsecutiveFrame::build()
                             ->seq_num(seq & 0xf)
                             ->data(frame_data)
                             ->build();
            if (!frame) {
                return {};
            }
            frames.push_back(frame.value());
        }
    } else {
        int len = payload.size();
        while (payload.size() < 7) payload.push_back(0);
        auto frame =
            Frame::SingleFrame::build()->len(len)->data(payload)->build();
        if (!frame) {
            return {};
        }
        frames.push_back(frame.value());
    }

    return frames;
}

Can::Transmitter::Transmitter(
    std::shared_ptr<Can::ServiceRequest::ServiceRequest> request) {
    if (request == nullptr) {
        m_status = Can::WorkerStatus::Done;
    }

    auto maybe_frames = Can::service_to_frames(request);
    if (!maybe_frames) {
        m_status = Can::WorkerStatus::Error;
        return;
    }
    m_frames = maybe_frames.value();
    switch (m_frames[0]->get_type()) {
        case Can::Frame::Type::SingleFrame: {
            m_status = Can::WorkerStatus::Work;
            DEBUG(info, "transmitter single frame");
            break;
        }
        case Can::Frame::Type::FirstFrame: {
            DEBUG(info, "transmitter first frame");
            m_status = Can::WorkerStatus::Work;
            m_i = 0;
            m_wait_fc = false;
            m_fc_min_time = static_cast<std::chrono::milliseconds>(0);
            break;
        }
        default:
            DEBUG(info, "transmitter error frame");
            m_status = Can::WorkerStatus::Error;
            break;
    }
}

void Can::Transmitter::push_frame(std::shared_ptr<Can::Frame::Frame> frame) {
    DEBUG(info, "transmitter pushing")
    if (m_wait_fc && frame->get_type() == Can::Frame::Type::FlowControl) {
        DEBUG(info, "transmitter pushing with m_wait_fc and flow control frame")
        m_wait_fc = false;
        Can::Frame::FlowStatus status =
            std::static_pointer_cast<Can::Frame::FlowControl>(frame)
                ->get_status();
        update_imp();
        if (status == Can::Frame::FlowStatus::
                          WaitForAnotherFlowControlMessageBeforeContinuing) {
            m_wait_fc = true;
            return;
        } else if (status ==
                   Can::Frame::FlowStatus::OverflowAbortTransmission) {
            m_status = Can::WorkerStatus::Error;
            return;
        }
        m_fc_block_size =
            std::static_pointer_cast<Can::Frame::FlowControl>(frame)
                ->get_block_size();
        if (m_fc_block_size == 0) m_fc_block_size = m_frames.size();
        m_fc_min_time = static_cast<std::chrono::milliseconds>(
            std::static_pointer_cast<Can::Frame::FlowControl>(frame)
                ->get_min_separation_time());
        m_last_frame_time = std::chrono::high_resolution_clock::now();
    }
}

optional<std::shared_ptr<Can::Frame::Frame>> Can::Transmitter::fetch_frame() {
    if (m_frames.size() == 1) {
        m_status = Can::WorkerStatus::Done;
        return m_frames[0];
    }

    if (m_wait_fc) {
        DEBUG(info, "transmitter fetch m_wait_fc");
        return {};
    } else {
        DEBUG(info, "transmitter fetch no m_wait_fc");
        if (m_i == 0) {
            m_i++;
            m_wait_fc = true;
            m_block_begin = 1;
            return m_frames[0];
        } else {
            if (std::chrono::duration_cast<std::chrono::milliseconds>(
                    std::chrono::high_resolution_clock::now() -
                    m_last_frame_time) < m_fc_min_time) {
                return {};
            }
            m_i++;
            if (m_i - m_block_begin + 1 >= m_fc_block_size) {
                m_wait_fc = true;
            }
            if (m_i == m_frames.size()) {
                m_status = Can::WorkerStatus::Done;
            }
            if (m_i > m_frames.size()) return {};
            m_last_frame_time = std::chrono::high_resolution_clock::now();
            return m_frames[m_i - 1];
        }
    }
}

Can::WorkerStatus Can::Transmitter::get_status() {
    if (m_wait_fc) {
        if (!check_timeout_imp()) {
            m_status = Can::WorkerStatus::Error;
        }
    }
    return m_status;
}

Can::Receiver::Receiver(std::shared_ptr<Can::Frame::Frame> frame) {
    switch (frame->get_type()) {
        case Can::Frame::Type::SingleFrame: {
            DEBUG(info, "receiver single frame");
            m_frames = {frame};
            m_status = Can::WorkerStatus::Done;
            m_was_fc = true;
            break;
        }
        case Can::Frame::Type::FirstFrame: {
            DEBUG(info, "receiver first frame");
            m_frames.push_back(frame);
            m_status = Can::WorkerStatus::Work;
            m_consecutive_len =
                std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                    ->get_len();
            m_consecutive_last = 0x0;
            m_was_fc = false;
            break;
        }
        default:
            DEBUG(info, "receiver error creating");
            m_status = Can::WorkerStatus::Error;
            break;
    }
}

optional<std::shared_ptr<Can::ServiceResponse::ServiceResponse>>
Can::Receiver::get_response() {
    DEBUG(info, "receiver");
    return Can::frames_to_service(m_frames);
}

void Can::Receiver::push_frame(std::shared_ptr<Can::Frame::Frame> frame) {
    switch (frame->get_type()) {
        case Can::Frame::Type::ConsecutiveFrame: {
            DEBUG(info, "pushing consecutive frame");
            m_consecutive_last = (m_consecutive_last + 1) & 0xf;
            m_frames.push_back(frame);
            if (m_consecutive_last !=
                std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                    ->get_seq_num()) {
                m_status = Can::WorkerStatus::Error;
                break;
            }
            update_imp();
            if (6 + (m_frames.size() - 1) * 7 >= m_consecutive_len) {
                m_status = Can::WorkerStatus::Done;
            }
            break;
        }
        default:
            m_status = Can::WorkerStatus::Error;
            break;
    }
}

optional<std::shared_ptr<Can::Frame::Frame>> Can::Receiver::fetch_frame() {
    if (m_was_fc) {
        DEBUG(info, "receiver m_was_fs");
        return {};
    }
    m_was_fc = true;
    update_imp();
    DEBUG(info, "receiver no m_was_fs");
    return Frame::FlowControl::build()
        ->status(Frame::FlowStatus::ContinueToSend)
        ->block_size(0)
        ->min_separation_time(0)
        ->build();
}

Can::WorkerStatus Can::Receiver::get_status() {
    if (!check_timeout_imp()) {
        m_status = Can::WorkerStatus::Error;
    }
    return m_status;
}
