#pragma once

#include <vector>
#include <memory>

#include "bytes.h"
#include "map.h"

namespace Can {

enum class FrameType;

class Frame {
public:
    virtual FrameType get_type() = 0;
    virtual std::vector<uint8_t> dump() = 0;
};

class FrameFactory {
public:
    FrameFactory(std::vector<uint8_t>);

    std::shared_ptr<Frame> get();

private:
#define FRAME(type, ...) std::shared_ptr<Frame> parse_##type();
#include "frame.inl"
#undef FRAME

    int m_offset;
    Util::Reader m_reader;
};

}  // namespace Can
