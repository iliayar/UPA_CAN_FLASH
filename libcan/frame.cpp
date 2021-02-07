#include "frame.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "can.h"

Can::FrameFactory::FrameFactory(std::vector<uint8_t> frame)
    : m_reader(frame), m_offset(0) {}

Can::Frame* Can::FrameFactory::get() {
    Can::FrameType frame_type =
	static_cast<Can::FrameType>(m_reader.read_8(m_offset, 4));
    m_offset += 4;

    switch (frame_type) {
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

    std::vector<uint8_t> data = m_reader.read(m_offset, len * 8);
    m_offset += len * 8;

    return new Frame_SingleFrame(len, data);
}

Can::Frame* Can::FrameFactory::parse_FirstFrame() {
    int len = m_reader.read_16(m_offset, 12);
    m_offset += 12;

    std::vector<uint8_t> data = m_reader.read(m_offset, 64 - m_offset);
    m_offset += 64 - m_offset;

    return new Frame_FirstFrame(len, data);
}
Can::Frame* Can::FrameFactory::parse_ConsecutiveFrame() {
    int seq_num = m_reader.read_8(m_offset, 4);
    m_offset += 4;

    std::vector<uint8_t> data = m_reader.read(m_offset, 64 - m_offset);
    m_offset += 8 - m_offset;

    return new Frame_ConsecutiveFrame(seq_num, data);
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

Can::Frame_SingleFrame::Frame_SingleFrame(int len, std::vector<uint8_t> data)
    : m_data(data), m_len(len) {}

Can::Frame_FirstFrame::Frame_FirstFrame(int len, std::vector<uint8_t> data)
    : m_data(data), m_len(len) {}

Can::Frame_ConsecutiveFrame::Frame_ConsecutiveFrame(int seq_num,
						    std::vector<uint8_t> data)
    : m_data(data), m_seq_num(seq_num) {}

Can::Frame_FlowControl::Frame_FlowControl(FlowStatus status, int block_size,
					  int min_separation_time)
    : m_status(status),
      m_block_size(block_size),
      m_min_separation_time(min_separation_time) {}

std::vector<uint8_t> Can::Frame_SingleFrame::dump() {
    std::vector<uint8_t> payload(8, 0);
    Writer writer(payload);

    int offset = 0;

    writer.write_8(static_cast<uint8_t>(FrameType::SingleFrame), offset, 4);
    offset += 4;

    writer.write_8(static_cast<uint8_t>(m_len), offset, 4);
    offset += 4;

    writer.write(m_data, offset, m_len * 8);
    offset += m_len * 8;

    return payload;
}

std::vector<uint8_t> Can::Frame_FirstFrame::dump() {
    std::vector<uint8_t> payload(8, 0);
    Writer writer(payload);

    int offset = 0;

    writer.write_8(static_cast<uint8_t>(FrameType::FirstFrame), offset, 4);
    offset += 4;

    writer.write_16(static_cast<uint16_t>(m_len), offset, 12);
    offset += 12;

    writer.write(m_data, offset, 64 - offset);
    offset += 64 - offset;

    return payload;
}

std::vector<uint8_t> Can::Frame_ConsecutiveFrame::dump() {
    std::vector<uint8_t> payload(8, 0);
    Writer writer(payload);

    int offset = 0;

    writer.write_8(static_cast<uint8_t>(FrameType::ConsecutiveFrame), offset,
		   4);
    offset += 4;

    writer.write_8(static_cast<uint8_t>(m_seq_num), offset, 4);
    offset += 4;

    writer.write(m_data, offset, 64 - offset);
    offset += 64 - offset;

    return payload;
}

std::vector<uint8_t> Can::Frame_FlowControl::dump() {
    std::vector<uint8_t> payload(8, 0);
    Writer writer(payload);

    int offset = 0;

    writer.write_8(static_cast<uint8_t>(FrameType::FlowControl), offset, 4);
    offset += 4;

    writer.write_8(static_cast<uint8_t>(m_status), offset, 4);
    offset += 4;

    writer.write_8(static_cast<uint8_t>(m_block_size), offset, 8);
    offset += 8;

    writer.write_8(static_cast<uint8_t>(m_min_separation_time), offset, 8);
    offset += 8;

    return payload;
}
