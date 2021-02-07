#pragma once

#include <vector>

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

    Frame* get();

private:
#define FRAME(type, ...) Frame* parse_##type();
#include "frame.inl"
#undef FRAME

    int m_offset;
    Util::Reader m_reader;
};

}  // namespace Can
