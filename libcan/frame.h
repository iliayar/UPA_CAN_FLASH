/*
 * @file frame.h
 * Only abstract frame class defined
 * here. All frame types defined in
 * {{ frame_all.h }}.
 */
#pragma once

#include <memory>
#include <vector>
#include <experimental/optional>

#include "bytes.h"
#include "objects.h"

using std::experimental::optional;

namespace Can {
namespace Frame {

/**
 * Enum class of frame types
 */
enum class FrameType {
    SingleFrame = 0x00,
    FirstFrame = 0x01,
    ConsecutiveFrame = 0x02,
    FlowControl = 0x03
};

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

protected:
    virtual void dump_impl(Util::Writer&) = 0;
};

class FrameFactory {
public:
    /**
     * Takes an 8 bytes vector to produce frame from
     * The one can be fetched using
     * @param data data to parse frame from
     */
    FrameFactory(std::vector<uint8_t> data);

    /**
     * @return optional of frame parsed from passed vector
     * in constructor. Returns null if these byte sequence
     * cannot be parsed in known frame type
     */
    optional<std::shared_ptr<Frame>> get();

private:

    Util::Reader m_reader;
    Util::EnumField<FrameType, uint8_t, 4> m_type;
};
}  // namespace Frame

}  // namespace Can
