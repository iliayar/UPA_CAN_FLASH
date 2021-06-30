/**
 * @file frame.cpp
 * This file is completely macros hell
 * Generated classes, parse and dump methods for
 * frame types defined in {{ frame.inl }}
 */
#include "frame.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "bytes.h"
#include "frame_all.h"

using namespace Can::Frame;

Factory::Factory(std::vector<uint8_t> const& frame)
    : m_reader(frame) {}

optional<std::shared_ptr<Frame>> Factory::get() {
    m_type.read(m_reader);
    auto frame_type = m_type.get();
    if(!frame_type) return {};

    switch (frame_type.value()) {
    case Type::SingleFrame:       
        return SingleFrame::build(m_reader)->build();
    case Type::FirstFrame:       
        return FirstFrame::build(m_reader)->build();
    case Type::ConsecutiveFrame:       
        return ConsecutiveFrame::build(m_reader)->build();
    case Type::FlowControl:       
        return FlowControl::build(m_reader)->build();
    }
    return {};
}
