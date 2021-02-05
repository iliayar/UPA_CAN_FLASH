#include "communicator.h"

#include <vector>
#include <stdexcept>

#include <iostream>

Can::Communicator::Communicator()
    : m_status(CommunicatorStatus::Ready)
{
}

void Can::Communicator::add_frame(Can::Frame* frame)
{
    Can::Frame_SingleFrame* single_frame;
    Can::Frame_FirstFrame* first_frame;
    Can::Frame_ConsecutiveFrame* consecutive_frame;
    Can::Frame_FlowControl* flow_control_frame;

    switch(frame->get_type()) {
    case Can::FrameType::SingleFrame:
        single_frame = static_cast<Can::Frame_SingleFrame*>(frame);
        m_data = single_frame->get_data();
        m_status = Can::CommunicatorStatus::Ready;
        break;
    case Can::FrameType::FirstFrame:
        first_frame = static_cast<Can::Frame_FirstFrame*>(frame);
        m_consecutive_last = 0xf;
        m_consecutive_len = first_frame->get_len();
        m_consecutive_read = 0;
        add_consecutive(first_frame->get_data(), 0);
        m_status = Can::CommunicatorStatus::Ready;
        break;
    case Can::FrameType::ConsecutiveFrame:
        consecutive_frame = static_cast<Can::Frame_ConsecutiveFrame*>(frame);
        add_consecutive(consecutive_frame->get_data(), consecutive_frame->get_seq_num());
        break;
    case Can::FrameType::FlowControl:
        flow_control_frame = static_cast<Can::Frame_FlowControl*>(frame);
        m_flow_status = flow_control_frame->get_status();
        m_flow_block_size = flow_control_frame->get_block_size();
        m_flow_min_time = flow_control_frame->get_min_separation_time();
    default:
        break;
    }

}

void Can::Communicator::add_consecutive(std::vector<uint8_t> data, int seq_num)
{
    m_status = Can::CommunicatorStatus::Waiting;
    for(uint8_t val : data) {
        m_data.push_back(val);
        m_consecutive_read++;
        if(m_consecutive_read >= m_consecutive_len) {
            m_status = Can::CommunicatorStatus::Ready;
            break;
        }
    }
    m_consecutive_last = (m_consecutive_last + 1) & 0x0f;
    if(m_consecutive_last != seq_num) {
        throw std::runtime_error("Invalid sequence numver met");
    }
}

Can::CommunicatorStatus Can::Communicator::get_status()
{
    return m_status;
}

Can::Frame* Can::Communicator::get_frame()
{
    return nullptr;
}

