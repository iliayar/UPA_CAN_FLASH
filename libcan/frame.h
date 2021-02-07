#pragma once

#include <cstdint>
#include <vector>

#include "bytes.h"
#include "map.h"

#define ARRAY std::vector<uint8_t>

namespace Can {

	
enum class FlowStatus {
    ContinueToSend = 0,
    WaitForAnotherFlowControlMessageBeforeContinuing = 1,
    OverflowAbortTransmission = 2
};

#define DUMP(...)
#define PARSE(...)

#define FRAME_CLASS(...)
#define FRAME(type, value) type = value
#define FRAMES(...) enum class FrameType { MAP_TUPLE_LIST(FRAME, __VA_ARGS__) };
#include "frame.inl"
#undef FRAME
#undef FRAMES
#undef FRAME_CLASS

class Frame {
public:
    virtual FrameType get_type() = 0;
    virtual std::vector<uint8_t> dump() = 0;
};

#define FRAMES(...)
#define FRAME_FIELD_GETTER(type, name) \
    type get_##name() { return m_##name; }
#define FRAME_FIELD(type, name) type m_##name;
#define FRAME_CTR_FIELD(type, name) type name
#define FRAME_CTR_FIELD_INIT(_, name) m_##name(name)
#define FRAME_CLASS(name, ...)                                     \
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
#undef FRAME_CLASS
#undef FRAME_CLASS_EMPTY
#undef FRAMES

class FrameFactory {
public:
    FrameFactory(std::vector<uint8_t>);

    Frame* get();

private:
#define FRAME_CLASS(...)
#define FRAME(type, _) Frame* parse_##type();
#define FRAMES(...) MAP_TUPLE(FRAME, __VA_ARGS__)
#include "frame.inl"
#undef FRAME_CLASS
#undef FRAME
#undef FRAMES
	
#undef DUMP
#undef PARSE

    int m_offset;
    Util::Reader m_reader;
};
}  // namespace Can
