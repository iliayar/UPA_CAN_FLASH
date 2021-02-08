#pragma once

#include <vector>

#include "bytes.h"
#include "service.h"
#include "map.h"

namespace Can {

#define SERVICE_BEGIN 

#define SUBFUNCION_ENUM_ELEM(type, value) type = value,
#define SUBFUNCTIONS(...)                            \
    enum class CONCAT(SERVICE, _SubfunctionType) {   \
	MAP_TUPLE(SUBFUNCION_ENUM_ELEM, __VA_ARGS__) \
    };
#define EXTRA
#include "services/services.h"
#undef EXTRA
#undef SUBFUNCTIONS
#undef SUBFUNCION_ENUM_ELEM

#undef SERVICE_BEGIN
// --------------- Service Request --------------------
#define SUBFUNCTIONS(...)
#define SERVICE_BEGIN SERVICE = REQUEST_ID,
enum class ServiceRequestType {
#include "services/services.h"
};
#undef SERVICE_BEGIN

#define SUBFUNCTION (CONCAT(SERVICE, _SubfunctionType), subfunction)
#define SERVICE_ARG(type, name) type name
#define SERVICE_ARG_INIT(_, name) m_##name(name)
#define SERVICE_FIELD(type, name) type m_##name;
#define SERVICE_BEGIN                                                         \
    class CONCAT(ServiceRequest_, SERVICE) : public ServiceRequest {          \
    public:                                                                   \
	CONCAT(ServiceRequest_, SERVICE)                                      \
	(MAP_TUPLE_LIST(SERVICE_ARG, REQUEST_FIELDS))                         \
	    : MAP_TUPLE_LIST(SERVICE_ARG_INIT, REQUEST_FIELDS) {}             \
        ServiceRequestType get_type() { return ServiceRequestType::SERVICE; } \
        std::vector<uint8_t> dump();                                          \
                                                                              \
    private:                                                                  \
        MAP_TUPLE(SERVICE_FIELD, REQUEST_FIELDS)                              \
    };
#include "services/services.h"
#undef SERVICE_ARG
#undef SERVICE_ARG_INIT
#undef SERVICE_FIELD
#undef SERVICE_BEGIN
#undef SUBFUNCTION
// --------------- Service Response -------------------

#define SERVICE_BEGIN SERVICE = RESPONSE_ID,
enum class ServiceResponseType {
#include "services/services.h"
	Negative = 0x7f
};
#undef SERVICE_BEGIN

#define SUBFUNCTION (CONCAT(SERVICE, _SubfunctionType), subfunction)
#define SERVICE_ARG(type, name) type name
#define SERVICE_ARG_INIT(_, name) m_##name(name)
#define SERVICE_FIELD(type, name) type m_##name;
#define SERVICE_GETTER(type, name) type get_##name() { return m_##name; };
#define SERVICE_BEGIN							\
	class CONCAT(ServiceResponse_, SERVICE) : public ServiceResponse { \
public: \
CONCAT(ServiceResponse_, SERVICE)(MAP_TUPLE_LIST(SERVICE_ARG, RESPONSE_FIELDS)) : MAP_TUPLE_LIST(SERVICE_ARG_INIT, RESPONSE_FIELDS) {} \
    ServiceResponseType get_type() { \
	return ServiceResponseType::SERVICE; \
    } \
    MAP_TUPLE(SERVICE_GETTER, RESPONSE_FIELDS) \
private: \
    MAP_TUPLE(SERVICE_FIELD, RESPONSE_FIELDS) \
};
#include "services/services.h"
#undef SERVICE_ARG
#undef SERVICE_ARG_INIT
#undef SERVICE_FIELD
#undef SERVICE_GETTER
#undef SERVICE_BEGIN
#undef SUBFUNCTIONS
#undef SUBFUNCTION

class ServiceResponse_Negative : public ServiceResponse {
public:
    ServiceResponse_Negative(ServiceRequestType service, uint8_t code)
	: m_service(service), m_code(code) {}

    ServiceResponseType get_type() { return ServiceResponseType::Negative; }

    ServiceRequestType get_service() { return m_service; }

    uint8_t get_code() { return m_code; }

private:
    ServiceRequestType m_service;
    uint8_t m_code;
};

}  // namespace Can
