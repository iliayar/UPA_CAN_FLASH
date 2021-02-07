#include "frame.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>

#include "bytes.h"

Can::FrameFactory::FrameFactory(std::vector<uint8_t> frame)
    : m_reader(frame), m_offset(0) {}

#define CASE(type)                   \
    case Can::FrameType::type:       \
	return this->parse_##type(); \
	break

Can::Frame* Can::FrameFactory::get() {
    Can::FrameType frame_type =
	static_cast<Can::FrameType>(m_reader.read_8(m_offset, 4));
    m_offset += 4;

    switch (frame_type) {
	CASE(SingleFrame);
	CASE(FirstFrame);
	CASE(ConsecutiveFrame);
	CASE(FlowControl);
    }
    return nullptr;
}

#undef CASE

#define PARSE_ENUM(name, type, n, len)                               \
    type name = static_cast<type>(m_reader.read_##n(m_offset, len)); \
    m_offset += len;
#define PARSE_INT(name, n, len)                          \
    uint##n##_t name = m_reader.read_##n(m_offset, len); \
    m_offset += len;
#define PARSE_VEC(name, len)                                  \
    std::vector<uint8_t> name = m_reader.read(m_offset, len); \
    m_offset += len;
#define PARSE_BEGIN(type) Can::Frame* Can::FrameFactory::parse_##type() {
#define PARSE_RETURN(type, ...)           \
    return new Frame_##type(__VA_ARGS__); \
    }
#define PARSE_ARG(func, ...) PARSE_##func(__VA_ARGS__)
#define PARSE_FETCH_NAME(_, name, ...) name
#define PARSE(type, ...)                    \
    PARSE_BEGIN(type)                       \
    EVAL(MAP_TUPLE(PARSE_ARG, __VA_ARGS__)) \
    PARSE_RETURN(type, MAP_TUPLE_LIST(PARSE_FETCH_NAME, __VA_ARGS__))

PARSE(SingleFrame, (INT, len, 8, 4), (VEC, data, len * 8))
PARSE(FirstFrame, (INT, len, 16, 12), (VEC, data, 64 - m_offset))
PARSE(ConsecutiveFrame, (INT, seq_num, 8, 4), (VEC, data, 64 - m_offset))
PARSE(FlowControl, (ENUM, status, FlowStatus, 8, 4), (INT, block_size, 8, 8),
      (INT, min_separation_time, 8, 8))
#undef PARSE_INT
#undef PARSE_VEC
#undef PARSE_BEGIN
#undef PARSE_RETURN

#define DUMP_INT(name, n, len)                                         \
    writer.write_##n(static_cast<uint##n##_t>(m_##name), offset, len); \
    offset += len;
#define DUMP_VEC(name, n)              \
    writer.write(m_##name, offset, n); \
    offset += n;
#define DUMP_BEGIN(type)                                                  \
    std::vector<uint8_t> Can::Frame_##type::dump() {                      \
	std::vector<uint8_t> payload(8, 0);                               \
	Util::Writer writer(payload);                                     \
	int offset = 0;                                                   \
	writer.write_8(static_cast<uint8_t>(FrameType::type), offset, 4); \
	offset += 4;
#define DUMP_END()  \
    return payload; \
    }
#define DUMP_ARG(func, ...) DUMP_##func(__VA_ARGS__)
#define DUMP(type, ...)                    \
    DUMP_BEGIN(type)                       \
    EVAL(MAP_TUPLE(DUMP_ARG, __VA_ARGS__)) \
    DUMP_END()
DUMP(SingleFrame, (INT, len, 8, 4), (VEC, data, m_len * 8))
DUMP(FirstFrame, (INT, len, 16, 12), (VEC, data, 64 - offset))
DUMP(ConsecutiveFrame, (INT, seq_num, 8, 4), (VEC, data, 64 - offset))
DUMP(FlowControl, (INT, status, 8, 4), (INT, block_size, 8, 8),
     (INT, min_separation_time, 8, 8))
#undef DUMP
#undef DUMP_BEGIN
#undef DUMP_END
#undef DUMP_INT
#undef DUMP_VEC
