/**
 * @file frame_all.h
 * Here defined classes of frame types
 * contains in {{ frame.inl }}
 */
#pragma once

#include <cstdint>
#include <vector>

#include "bytes.h"
#include "frame.h"
#include "map.h"

#define ARRAY std::vector<uint8_t>

namespace Can {

enum class FlowStatus {
    ContinueToSend = 0,
    WaitForAnotherFlowControlMessageBeforeContinuing = 1,
    OverflowAbortTransmission = 2
};

#define FRAME(type, value, ...) type = value,
/**
 * Enum class of frame types defined in {{ frame.inl }}
 */
enum class FrameType {
#include "frame.inl"
};
#undef FRAME

#define TYPE_INT(...) int
#define TYPE_VEC(...) std::vector<uint8_t>
#define TYPE_ENUM(type, ...) type
#define FRAME_FIELD_GETTER(type, name, ...) \
    TYPE_##type(__VA_ARGS__) get_##name() { return m_##name; }
#define FRAME_FIELD(type, name, ...) TYPE_##type(__VA_ARGS__) m_##name;
#define FRAME_CTR_FIELD(type, name, ...) TYPE_##type(__VA_ARGS__) name
#define FRAME_CTR_FIELD_INIT(_, name, ...) m_##name(name)
#define FRAME(name, _, ...)                                        \
    class Frame_##name : public Frame {                            \
    public:                                                        \
	Frame_##name(MAP_TUPLE_LIST(FRAME_CTR_FIELD, __VA_ARGS__)) \
	    : MAP_TUPLE_LIST(FRAME_CTR_FIELD_INIT, __VA_ARGS__) {} \
	FrameType get_type() { return FrameType::name; }           \
	MAP_TUPLE(FRAME_FIELD_GETTER, __VA_ARGS__)                 \
	std::vector<uint8_t> dump();                               \
								   \
    private:                                                       \
	MAP_TUPLE(FRAME_FIELD, __VA_ARGS__)                        \
    };
#include "frame.inl"
#undef FRAME_FIELD_GETTER
#undef FRAME_FIELD
#undef FRAME_CTR_FIELD
#undef FRAME_CTR_FIELD_INIT
#undef FRAME
#undef TYPE_INT
#undef TYPE_VEC
#undef TYPE_ENUM
}  // namespace Can
