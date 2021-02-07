#pragma once

#include <cstdint>
#include <vector>

#include "bytes.h"
#include "map.h"

namespace Can {

enum class FrameType {
    SingleFrame = 0,
    FirstFrame = 1,
    ConsecutiveFrame = 2,
    FlowControl = 3
};

enum class FlowStatus {
    ContinueToSend = 0,
    WaitForAnotherFlowControlMessageBeforeContinuing = 1,
    OverflowAbortTransmission = 2
};

class Frame {
public:
    virtual FrameType get_type() = 0;
    virtual std::vector<uint8_t> dump() = 0;
};

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
    }
FRAME_CLASS(SingleFrame, (int, len), (std::vector<uint8_t>, data));
FRAME_CLASS(FirstFrame, (int, len), (std::vector<uint8_t>, data));
FRAME_CLASS(ConsecutiveFrame, (int, seq_num), (std::vector<uint8_t>, data));
FRAME_CLASS(FlowControl, (FlowStatus, status), (int, block_size),
	    (int, min_separation_time));
#undef FRAME_FIELD_GETTER
#undef FRAME_FIELD
#undef FRAME_CTR_FIELD
#undef FRAME_CTR_FIELD_INIT
#undef FRAME_CLASS
#undef FRAME_CLASS_EMPTY

class FrameFactory {
public:
    FrameFactory(std::vector<uint8_t>);

    Frame* get();

private:
    Frame* parse_SingleFrame();
    Frame* parse_FirstFrame();
    Frame* parse_ConsecutiveFrame();
    Frame* parse_FlowControl();

    int m_offset;
    Util::Reader m_reader;
};
}  // namespace Can
