#include "service.h"

#include <stdexcept>
#include <memory>

#include "bytes.h"
#include "service_all.h"

// ---------- Service Request --------------
#define SUBFUNCTIONS(...)
#define SERVICE_BEGIN

#define ALL 64 - offset

#define PARSE_ENUM(name, type, len)                                     \
	type m_##name = static_cast<type>(reader.read_64(offset, len)); \
	offset += len;
#define PARSE_INT(name, len)                             \
	uint64_t m_##name = reader.read_64(offset, len);	\
	offset += len;
#define PARSE_VEC(name, len)                                    \
    std::vector<uint8_t> m_##name = reader.read(offset, len); \
    offset += len;
#define PARSE_BEGIN(type)                                               \
    std::shared_ptr<Can::type> Can::type::parse(Util::Reader& reader) { \
        int offset = 0;
#define PARSE_RETURN(type, ...)                      \
    reader.add_offset(offset);                       \
    return std::make_shared<Can::type>(__VA_ARGS__); \
    }
#define PARSE_ARG(func, ...) PARSE_##func(__VA_ARGS__)
#define PARSE_FETCH_NAME(_, name, ...) m_##name
#define DATATYPE(type, ...)		    \
	PARSE_BEGIN(type)		    \
	EVAL(MAP_TUPLE(PARSE_ARG, __VA_ARGS__))				\
	PARSE_RETURN(type, MAP_TUPLE_LIST(PARSE_FETCH_NAME, __VA_ARGS__))
#include "services/services.h"
#undef PARSE_INT
#undef PARSE_VEC
#undef PARSE_BEGIN
#undef PARSE_RETURN
#undef DATATYPE

#define DUMP_INT(name, len)                                          \
    writer.write_64(static_cast<uint64_t>(m_##name), offset, len); \
    offset += len;
#define DUMP_ENUM(name, _, len) DUMP_INT(name, len)
#define DUMP_VEC(name, len)                                             \
    while (m_##name.size() < ((len) + 7) / 8) m_##name.push_back(0x00); \
    writer.write(m_##name, offset, len);                              \
    offset += len;
#define DUMP_BEGIN(type)                                                  \
    std::vector<uint8_t> Can::type::dump() {                              \
        std::vector<uint8_t> payload(8, 0);                               \
        Util::Writer writer(payload);                                     \
        int offset = 0;
#define DUMP_END()  \
    return payload; \
    }
#define DUMP_ARG(func, ...) DUMP_##func(__VA_ARGS__)
#define DATATYPE(type, ...)		   \
	DUMP_BEGIN(type)		   \
	EVAL(MAP_TUPLE(DUMP_ARG, __VA_ARGS__))	\
	DUMP_END()
#include "services/services.h"
#undef DATATYPE
#undef DUMP_BEGIN
#undef DUMP_END
#undef DUMP_INT
#undef DUMP_VEC
#undef DUMP_ENUM
#undef ALL

#undef SERVICE_BEGIN
#define DATATYPE(...)

Can::ServiceResponseType Can::request_to_response_type(
    Can::ServiceRequestType type) {
    switch (type) {
#define SERVICE_BEGIN                      \
    case Can::ServiceRequestType::SERVICE: \
        return Can::ServiceResponseType::SERVICE;
#include "services/services.h"
#undef SERVICE_BEGIN
        default:
            return Can::ServiceResponseType::Negative;
    }
}

#define RETURN return payload
#define INIT                                                               \
    std::vector<uint8_t> payload(1, 0);                                    \
    Util::Writer writer(payload);                                          \
    int offset = 0;                                                        \
    writer.write_8(static_cast<uint8_t>(Can::ServiceRequestType::SERVICE), \
                   offset, 8);                                             \
    offset += 8;
#define FIELD_SUBFUNCTION()                                         \
    payload.resize(payload.size() + 1, 0);                          \
    writer.write_8(static_cast<uint8_t>(m_subfunction), offset, 8); \
    offset += 8;                                                    \
    switch (m_subfunction)
#define CASE(name) case CONCAT(Can::SERVICE, _SubfunctionType)::name:
#define FIELD_DATA(name)                              \
    {                                                 \
        std::vector<uint8_t> payload_t = name->dump(); \
        FIELD_VEC(payload_t, payload_t.size());       \
    }
#define FIELD_VEC(value, len)                          \
    payload.resize(payload.size() + (len + 7) / 8, 0); \
    writer.write(value, offset, len);                  \
    offset += len;
#define FIELD_INT(value, len)                                   \
    payload.resize(payload.size() + (len + 7) / 8, 0);          \
    writer.write_64(static_cast<uint64_t>(value), offset, len); \
    offset += len;
#define FIELD(func, ...) FIELD_##func(__VA_ARGS__)
#define DUMP
#define SERVICE_BEGIN \
    std::vector<uint8_t> CONCAT(Can::ServiceRequest_, SERVICE)::dump()
#include "services/services.h"
#undef SERVICE_BEGIN
#undef DUMP
#undef FIELD
#undef FIELD_ENUM
#undef FIELD_VEC
#undef FIELD_INT
#undef FIELD_DATA
#undef INIT
#undef RETURN
#undef CASE
#undef FIELD_SUBFUNCTION

// ---------- Service Response -------------
Can::ServiceResponseFactory::ServiceResponseFactory(
    std::vector<uint8_t> payload)
    : m_offset(0), m_reader(payload), m_size(payload.size()*8) {}

#define ALL m_size - m_offset
#define RETURN(...) \
    return new CONCAT(Can::ServiceResponse_, SERVICE)(__VA_ARGS__);
#define FIELD_VEC(name, len)                                  \
    std::vector<uint8_t> name = m_reader.read(m_offset, len); \
    m_offset += len;
#define FIELD_SUBFUNCTION()                                            \
    CONCAT(Can::SERVICE, _SubfunctionType)                             \
    subfunction = static_cast<CONCAT(Can::SERVICE, _SubfunctionType)>( \
        m_reader.read_8(m_offset, 8));                                 \
    m_offset += 8;                                                     \
    switch (subfunction)
#define CASE(name) case CONCAT(Can::SERVICE, _SubfunctionType)::name:
#define FIELD_ENUM(name, type, len)                                           \
    Can::type name = static_cast<Can::type>(m_reader.read_64(m_offset, len)); \
    m_offset += len;
#define FIELD_DATA(name, type)                   \
    m_reader.add_offset(m_offset);               \
    std::shared_ptr<Can::type> name = Can::type::parse(m_reader); \
    m_offset = 0;                                \
    m_size -= m_reader.get_offset();
#define FIELD_INT(name, len)                         \
    uint64_t name = m_reader.read_64(m_offset, len); \
    m_offset += len;
#define FIELD(func, ...) FIELD_##func(__VA_ARGS__)

#define PARSE
#define SERVICE_BEGIN \
    Can::ServiceResponse* CONCAT(Can::ServiceResponseFactory::parse_, SERVICE)()
#include "services/services.h"
#undef SERVICE_BEGIN
#undef PARSE

#define SERVICE Negative
Can::ServiceResponse* Can::ServiceResponseFactory::parse_Negative() {
    FIELD(ENUM, service, ServiceRequestType, 8);
    FIELD(INT, code, 8);
    RETURN(service, code);
}
#undef SERVICE

#undef FIELD
#undef FIELD_ENUM
#undef FIELD_INT
#undef FIELD_VEC
#undef FIELD_DATA
#undef ALL
#undef FIELD_SUBFUNCTION
#undef CASE
#undef RETURN

Can::ServiceResponse* Can::ServiceResponseFactory::get() {
    ServiceResponseType type =
        static_cast<ServiceResponseType>(m_reader.read_8(m_offset, 8));
    m_offset += 8;

    switch (type) {
#define SERVICE_BEGIN                       \
    case Can::ServiceResponseType::SERVICE: \
        return CONCAT(parse_, SERVICE)();
#include "services/services.h"
#undef SERVICE_BEGIN
        case Can::ServiceResponseType::Negative:
            return parse_Negative();
        default:
            return nullptr;
    }
#undef SUBFUNCTIONS
#undef DATATYPE
}
