#include "frame.h"
#include "can.h"

#include <cstdint>
#include <stdexcept>

#include <iostream>

Can::FrameFactory::FrameFactory(std::vector<uint8_t> frame)
    : m_reader(frame)
    , m_offset(0)
{}

Can::Frame* Can::FrameFactory::get() {
    Can::FrameType frame_type = static_cast<Can::FrameType>(m_reader.read_8(m_offset, 4));
    m_offset += 4;
    
    switch(frame_type) {
        case Can::FrameType::SingleFrame:
            return this->parse_SingleFrame();
            break;
        case Can::FrameType::FirstFrame:
            return this->parse_FirstFrame();
            break;
        case Can::FrameType::ConsecutiveFrame:
            return this->parse_ConsecutiveFrame();
            break;
        case Can::FrameType::FlowControl:
            return this->parse_FlowControl();
            break;
    }
    return nullptr;
}


Can::Frame* Can::FrameFactory::parse_SingleFrame() {
    int len = m_reader.read_8(m_offset, 4);
    m_offset += 4;

    std::vector<uint8_t> data = m_reader.read(m_offset, len*8);
    m_offset += len*8;

    return new Frame_SingleFrame(data, len);
}

Can::Frame* Can::FrameFactory::parse_FirstFrame() {
    int len = m_reader.read_16(m_offset, 12);
    m_offset += 12;

    std::vector<uint8_t> data = m_reader.read(m_offset, 64 - m_offset);
    m_offset += 64 - m_offset;

    return new Frame_FirstFrame(data, len);
}
Can::Frame* Can::FrameFactory::parse_ConsecutiveFrame() {
    int seq_num = m_reader.read_8(m_offset, 4);
    m_offset += 4;

    std::vector<uint8_t> data = m_reader.read(m_offset, 64 - m_offset);
    m_offset += 8 - m_offset;

    return new Frame_ConsecutiveFrame(data, seq_num);
}
Can::Frame* Can::FrameFactory::parse_FlowControl() {
    FlowStatus status = static_cast<FlowStatus>(m_reader.read_8(m_offset, 4));
    m_offset += 4;

    uint8_t block_size = m_reader.read_8(m_offset, 8);
    m_offset += 8;

    uint8_t min_separation_time = m_reader.read_8(m_offset, 8);
    m_offset += 8;

    return new Frame_FlowControl(status, block_size, min_separation_time);
}

Can::Frame_SingleFrame::Frame_SingleFrame(std::vector<uint8_t> data, int len)
    : m_data(data)
    , m_len(len)
{}

Can::Frame_FirstFrame::Frame_FirstFrame(std::vector<uint8_t> data, int len)
    : m_data(data)
    , m_len(len)
{}

Can::Frame_ConsecutiveFrame::Frame_ConsecutiveFrame(std::vector<uint8_t> data, int seq_num)
    : m_data(data)
    , m_seq_num(seq_num)
{}

Can::Frame_FlowControl::Frame_FlowControl(FlowStatus status, int block_size, int min_separation_time)
    : m_status(status)
    , m_block_size(block_size)
    , m_min_separation_time(min_separation_time)
{}
