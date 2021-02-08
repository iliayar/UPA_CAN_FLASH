#include "service.h"
#include "service_all.h"
#include "bytes.h"

#include <stdexcept>


// ---------- Service Request --------------
#define SUBFUNCTIONS(...)
#define RETURN return payload
#define INIT                                                               \
    std::vector<uint8_t> payload(1, 0);                                    \
    Util::Writer writer(payload);                                          \
    int offset = 0;                                                        \
    writer.write_8(static_cast<uint8_t>(Can::ServiceRequestType::SERVICE), \
		   offset, 8);                                             \
    offset += 8;
#define FIELD_SUBFUNCTION()					      \
    payload.resize(payload.size() + 1, 0);                            \
    writer.write_8(static_cast<uint8_t>(m_subfunction), offset, 8);   \
    offset += 8;						      \
    switch (m_subfunction)
#define CASE(name) case CONCAT(Can::SERVICE, _SubfunctionType)::name:
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
#define SERVICE_BEGIN std::vector<uint8_t> CONCAT(Can::ServiceRequest_, SERVICE)::dump()
#include "services/services.h"
#undef SERVICE_BEGIN
#undef DUMP
#undef FIELD
#undef FIELD_ENUM
#undef FIELD_VEC
#undef FIELD_INT
#undef INIT
#undef RETURN
#undef CASE
#undef FIELD_SUBFUNCTION

// ---------- Service Response -------------
Can::ServiceResponseFactory::ServiceResponseFactory(
    std::vector<uint8_t> payload)
    : m_offset(0), m_reader(payload) {}

#define RETURN(...) \
    return new CONCAT(Can::ServiceResponse_, SERVICE)(__VA_ARGS__);
#define FIELD_SUBFUNCTION()					       \
    CONCAT(Can::SERVICE, _SubfunctionType)                             \
    subfunction = static_cast<CONCAT(Can::SERVICE, _SubfunctionType)>( \
	m_reader.read_8(m_offset, 8));                                 \
    m_offset += 8;                                                     \
    switch (subfunction)
#define CASE(name) case CONCAT(Can::SERVICE, _SubfunctionType)::name:
#define FIELD_ENUM(name, type, len)                                           \
    Can::type name = static_cast<Can::type>(m_reader.read_64(m_offset, len)); \
    m_offset += len;
#define FIELD_INT(name, len)                         \
    uint64_t name = m_reader.read_64(m_offset, len); \
    m_offset += len;
#define FIELD(func, ...) FIELD_##func(__VA_ARGS__)

#define PARSE
#define SERVICE_BEGIN  Can::ServiceResponse* CONCAT(Can::ServiceResponseFactory::parse_, SERVICE)()
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
}
