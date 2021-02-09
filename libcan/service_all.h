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
#define TYPE_INT(...) int
#define TYPE_VEC(...) std::vector<uint8_t>
#define TYPE_ENUM(type, ...) type
#define DATATYPE_FIELD_GETTER(type, name, ...) \
    TYPE_##type(__VA_ARGS__) get_##name() { return m_##name; }
#define DATATYPE_FIELD(type, name, ...) TYPE_##type(__VA_ARGS__) m_##name;
#define DATATYPE_CTR_FIELD(type, name, ...) TYPE_##type(__VA_ARGS__) name
#define DATATYPE_CTR_FIELD_INIT(_, name, ...) m_##name(name)
#define DATATYPE(name, ...)                                           \
    class name {                                                      \
    public:                                                           \
        static std::shared_ptr<name> parse(Util::Reader&);                             \
        name(MAP_TUPLE_LIST(DATATYPE_CTR_FIELD, __VA_ARGS__))         \
            : MAP_TUPLE_LIST(DATATYPE_CTR_FIELD_INIT, __VA_ARGS__) {} \
        MAP_TUPLE(DATATYPE_FIELD_GETTER, __VA_ARGS__)                 \
        std::vector<uint8_t> dump();                                  \
                                                                      \
    private:                                                          \
        MAP_TUPLE(DATATYPE_FIELD, __VA_ARGS__)                        \
    };
#define EXTRA
#include "services/services.h"
#undef EXTRA
#undef SUBFUNCTIONS
#undef SUBFUNCION_ENUM_ELEM
#undef DATATYPE_FIELD_GETTER
#undef DATATYPE_FIELD
#undef DATATYPE_CTR_FIELD
#undef DATATYPE_CTR_FIELD_INIT
#undef DATATYPE
#undef TYPE_INT
#undef TYPE_VEC
#undef TYPE_ENUM

#undef SERVICE_BEGIN
#define DATATYPE(...)
#define SUBFUNCTIONS(...)
    
// --------------- Service Request --------------------
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

#define SUBFUNCTION (CONCAT(SERVICE, _SubfunctionType), subfunction)
#define SERVICE_SETTER(type, name) \
    void name(type value) { m_##name = value; }
#define SERVICE_ARG_INIT(_, name) m_##name
#define SERVICE_FIELD(type, name) type m_##name;
#define SERVICE_BEGIN                                              \
    class CONCAT(CONCAT(ServiceRequest_, SERVICE), _Builder) {     \
    public:                                                        \
        CONCAT(ServiceRequest_, SERVICE) * build() {               \
            return new CONCAT(ServiceRequest_, SERVICE)(           \
                MAP_TUPLE_LIST(SERVICE_ARG_INIT, REQUEST_FIELDS)); \
        }                                                          \
        MAP_TUPLE(SERVICE_SETTER, REQUEST_FIELDS)                  \
    private:                                                       \
        MAP_TUPLE(SERVICE_FIELD, REQUEST_FIELDS)                   \
    };
#include "services/services.h"
#undef SERVICE_SETTER
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
#undef DATATYPE

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
