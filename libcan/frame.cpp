#include "frame_all.h"
#include "bytes.h"
#include "frame.h"

#include <cstdint>
#include <iostream>
#include <stdexcept>


Can::FrameFactory::FrameFactory(std::vector<uint8_t> frame)
	: m_reader(frame), m_offset(0) {}

#define FRAME(type, ...)	     \
	case Can::FrameType::type:   \
	return this->parse_##type(); \
	break;

std::shared_ptr<Can::Frame> Can::FrameFactory::get() {
	Can::FrameType frame_type =
		static_cast<Can::FrameType>(m_reader.read_8(m_offset, 4));
	m_offset += 4;

	switch (frame_type) {
#include "frame.inl"
	}
	return nullptr;
}

#undef FRAME

#define ALL 64 - m_offset

#define PARSE_ENUM(name, type, len)                                     \
	type m_##name = static_cast<type>(m_reader.read_64(m_offset, len)); \
	m_offset += len;
#define PARSE_INT(name, len)                             \
	uint64_t m_##name = m_reader.read_64(m_offset, len);	\
	m_offset += len;
#define PARSE_VEC(name, len)                                      \
	std::vector<uint8_t> m_##name = m_reader.read(m_offset, len);	\
	m_offset += len;
#define PARSE_BEGIN(type) std::shared_ptr<Can::Frame> Can::FrameFactory::parse_##type() {
#define PARSE_RETURN(type, ...)           \
	return std::make_shared<Frame_##type>(__VA_ARGS__);	\
	}
#define PARSE_ARG(func, ...) PARSE_##func(__VA_ARGS__)
#define PARSE_FETCH_NAME(_, name, ...) m_##name
#define FRAME(type, _, ...)		    \
	PARSE_BEGIN(type)		    \
	EVAL(MAP_TUPLE(PARSE_ARG, __VA_ARGS__))				\
	PARSE_RETURN(type, MAP_TUPLE_LIST(PARSE_FETCH_NAME, __VA_ARGS__))
#include "frame.inl"
#undef PARSE_INT
#undef PARSE_VEC
#undef PARSE_BEGIN
#undef PARSE_RETURN
#undef FRAME

#define DUMP_INT(name, len)                                          \
	writer.write_64(static_cast<uint64_t>(m_##name), m_offset, len); \
	m_offset += len;
#define DUMP_ENUM(name, _, len) DUMP_INT(name, len)
#define DUMP_VEC(name, len)                                             \
	while (m_##name.size() < ((len) + 7) / 8) m_##name.push_back(0x00); \
	writer.write(m_##name, m_offset, len);				\
	m_offset += len;
#define DUMP_BEGIN(type)						\
	std::vector<uint8_t> Can::Frame_##type::dump() {		\
	std::vector<uint8_t> payload(8, 0);				\
	Util::Writer writer(payload);					\
	int m_offset = 0;						\
	writer.write_8(static_cast<uint8_t>(FrameType::type), m_offset, 4); \
	m_offset += 4;
#define DUMP_END()  \
	return payload;				\
	}
#define DUMP_ARG(func, ...) DUMP_##func(__VA_ARGS__)
#define FRAME(type, _, ...)		   \
	DUMP_BEGIN(type)		   \
	EVAL(MAP_TUPLE(DUMP_ARG, __VA_ARGS__))	\
	DUMP_END()
#include "frame.inl"
#undef FRAME
#undef DUMP_BEGIN
#undef DUMP_END
#undef DUMP_INT
#undef DUMP_VEC
#undef DUMP_ENUM

#undef ALL
