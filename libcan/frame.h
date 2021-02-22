/*
 * @file frame.h
 * Only abstract frame class defined
 * here. All frame types defined in
 * {{ frame_all.h }}.
 */
#pragma once

#include <vector>
#include <memory>

#include "bytes.h"
#include "map.h"

namespace Can {

enum class FrameType;

class Frame {
public:

    /**
     * @return enum type of frame
     */
    virtual FrameType get_type() = 0;

    /**
     * Converts frame to raw bytes
     * @return 8 bytes representation of frame
     */
    virtual std::vector<uint8_t> dump() = 0;
};

class FrameFactory {
public:
    /**
     * Takes an 8 bytes vector to produce frame from
     * The one can be fetched using 
     */
    FrameFactory(std::vector<uint8_t>);

    /**
     * @return frame parsed from passed vector
     * in constructor. Returns null if these byte sequence
     * cannot be parsed in known frame type
     */
    std::shared_ptr<Frame> get();

private:
#define FRAME(type, ...) std::shared_ptr<Frame> parse_##type();
#include "frame.inl"
#undef FRAME

    int m_offset;
    Util::Reader m_reader;
};

}  // namespace Can
