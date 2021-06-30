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
enum class Type {
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
    virtual Type get_type() = 0;

    /**
     * Converts frame to raw bytes
     * @return 8 bytes representation of frame
     */
    optional<std::vector<uint8_t>> dump() {
        Util::Writer writer(8);
        writer.write_int<uint8_t>(static_cast<uint8_t>(get_type()), 4);
        if(!dump_impl(writer)) {
            return {};
        }
        return writer.get_payload();
    }

    virtual ~Frame() {}

protected:
    virtual bool dump_impl(Util::Writer&) = 0;
};

class Factory {
public:
    /**
     * Takes an 8 bytes vector to produce frame from
     * The one can be fetched using
     * @param data data to parse frame from
     */
    Factory(std::vector<uint8_t> data);

    /**
     * @return optional of frame parsed from passed vector
     * in constructor. Returns null if these byte sequence
     * cannot be parsed in known frame type
     */
    optional<std::shared_ptr<Frame>> get();

private:

    Util::Reader m_reader;
    Util::EnumField<Type, uint8_t, 4> m_type;
};
}  // namespace Frame

}  // namespace Can
