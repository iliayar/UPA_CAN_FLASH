#pragma once

#include <vector>

#include "bytes.h"
#include "map.h"
#include "service.h"


// Macros magick

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
#define TYPE_DATA(type, ...) std::shared_ptr<type>
#define DATATYPE_FIELD_GETTER(type, name, ...) \
    TYPE_##type(__VA_ARGS__) get_##name() { return m_##name; }
#define DATATYPE_FIELD(type, name, ...) TYPE_##type(__VA_ARGS__) m_##name;
#define DATATYPE_FIELD_SETTER(type, name, ...)            \
    auto name(TYPE_##type(__VA_ARGS__) value) { \
        m_##name = value;                                 \
        return this;                                      \
    }
#define DATATYPE_CTR_FIELD(type, name, ...) TYPE_##type(__VA_ARGS__) name
#define DATATYPE_CTR_FIELD_INIT(_, name, ...) m_##name(name)
#define DATATYPE_CTR_FIELD_ARG(_, name, ...) m_##name
#define DATATYPE(name, ...)                                           \
    class name##_Builder;                                             \
    class name {                                                      \
    public:                                                           \
        static std::shared_ptr<name> parse(Util::Reader&);            \
        name(MAP_TUPLE_LIST(DATATYPE_CTR_FIELD, __VA_ARGS__))         \
            : MAP_TUPLE_LIST(DATATYPE_CTR_FIELD_INIT, __VA_ARGS__) {} \
        MAP_TUPLE(DATATYPE_FIELD_GETTER, __VA_ARGS__)                 \
        std::vector<uint8_t> dump(int*);                    \
        static std::unique_ptr<name##_Builder> build() {              \
            return std::make_unique<name##_Builder>();                \
        }                                                             \
                                                                      \
    private:                                                          \
        friend class name##_Builder;                                  \
        MAP_TUPLE(DATATYPE_FIELD, __VA_ARGS__)                        \
    };                                                                \
    class name##_Builder {                                            \
    public:                                                           \
        std::shared_ptr<name> build() {                               \
            return std::make_shared<name>(                            \
                MAP_TUPLE_LIST(DATATYPE_CTR_FIELD_ARG, __VA_ARGS__)); \
        }                                                             \
        MAP_TUPLE(DATATYPE_FIELD_SETTER, __VA_ARGS__)                 \
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
#undef DATATYPE_FIELD_SETTER
#undef DATATYPE_FIELD
#undef DATATYPE_CTR_FIELD
#undef DATATYPE_CTR_FIELD_INIT
#undef DATATYPE_CTR_FIELD_ARG
#undef DATATYPE
#undef TYPE_INT
#undef TYPE_VEC
#undef TYPE_ENUM
#undef TYPE_DATA

#undef SERVICE_BEGIN
#define DATATYPE(...)
#define SUBFUNCTIONS(...)

// --------------- Service Request --------------------
#define SERVICE_BEGIN SERVICE = REQUEST_ID,
enum class ServiceRequestType {
#include "services/services.h"
};
#undef SERVICE_BEGIN

#define FIELD_INT(n) uint##n##_t
#define FIELD_RAW(a) a
#define FIELD_DATA(a) std::shared_ptr<a>
#define FIELD_VEC() std::vector<uint8_t>

#define DESTRUCT_INT(...)
#define DESTRUCT_RAW(...)
// #define DESTRUCT_DATA(name, ...) delete m_##name;
#define DESTRUCT_DATA(name, ...)
#define DESTRUCT_VEC(...)
#define DESTRUCT(type, name, ...) DESTRUCT_##type(name, __VA_ARGS__)

#define SUBFUNCTION (RAW, subfunction, CONCAT(SERVICE, _SubfunctionType))
#define SERVICE_ARG(type, name, ...) FIELD_##type(__VA_ARGS__) name
#define SERVICE_ARG_INIT(_, name, ...) m_##name(name)
#define SERVICE_FIELD(type, name, ...) FIELD_##type(__VA_ARGS__) m_##name;
#define SERVICE_BEGIN                                                         \
    class CONCAT(CONCAT(ServiceRequest_, SERVICE), _Builder);                 \
    class CONCAT(ServiceRequest_, SERVICE) : public ServiceRequest {          \
    public:                                                                   \
        CONCAT(ServiceRequest_, SERVICE)                                      \
        (MAP_TUPLE_LIST(SERVICE_ARG, REQUEST_FIELDS))                         \
            : MAP_TUPLE_LIST(SERVICE_ARG_INIT, REQUEST_FIELDS) {}             \
        ServiceRequestType get_type() { return ServiceRequestType::SERVICE; } \
        std::vector<uint8_t> dump();                                          \
        static std::unique_ptr<CONCAT(CONCAT(ServiceRequest_, SERVICE),       \
                                      _Builder)>                              \
        build() {                                                             \
            return std::make_unique<CONCAT(CONCAT(ServiceRequest_, SERVICE),  \
                                           _Builder)>();                      \
        }                                                                     \
        ~CONCAT(ServiceRequest_, SERVICE)() {                                 \
            MAP_TUPLE(DESTRUCT, REQUEST_FIELDS)                               \
        }                                                                     \
                                                                              \
    private:                                                                  \
        friend class CONCAT(CONCAT(ServiceRequest_, SERVICE), _Builder);      \
        MAP_TUPLE(SERVICE_FIELD, REQUEST_FIELDS)                              \
    };
#include "services/services.h"
#undef SERVICE_ARG
#undef SERVICE_ARG_INIT
#undef SERVICE_FIELD
#undef SERVICE_BEGIN
#undef SUBFUNCTION

#undef DESTRUCT_INT
#undef DESTRUCT_RAW
#undef DESTRUCT_DATA
#undef DESTRUCT_VEC
#undef DESTRUCT

#define SUBFUNCTION (RAW, subfunction, CONCAT(SERVICE, _SubfunctionType))
#define SERVICE_SETTER(type, name, ...)                  \
    CONCAT(CONCAT(ServiceRequest_, SERVICE), _Builder) * \
        name(FIELD_##type(__VA_ARGS__) value) {          \
        m_##name = value;                                \
        return this;                                     \
    }
#define SERVICE_ARG_INIT(_, name, ...) m_##name
#define SERVICE_FIELD(type, name, ...) FIELD_##type(__VA_ARGS__) m_##name;
#define SERVICE_BEGIN                                                   \
    class CONCAT(CONCAT(ServiceRequest_, SERVICE), _Builder) {          \
    public:                                                             \
        std::shared_ptr<CONCAT(ServiceRequest_, SERVICE)> build() {     \
            return std::make_shared<CONCAT(ServiceRequest_, SERVICE)>(  \
                MAP_TUPLE_LIST(SERVICE_ARG_INIT, REQUEST_FIELDS));      \
        }                                                               \
        MAP_TUPLE(SERVICE_SETTER, REQUEST_FIELDS)                       \
            private:                                                    \
            MAP_TUPLE(SERVICE_FIELD, REQUEST_FIELDS)                    \
    };
#include "services/services.h"
#undef SERVICE_SETTER
#undef SERVICE_ARG_INIT
#undef SERVICE_FIELD
#undef SERVICE_BEGIN
#undef SUBFUNCTION
#undef FIELD_DATA

// --------------- Service Response -------------------

#define SERVICE_BEGIN SERVICE = RESPONSE_ID,
enum class ServiceResponseType {
#include "services/services.h"
    Negative = 0x7f
};
#undef SERVICE_BEGIN

#define FIELD_DATA(a) std::shared_ptr<a>

#define SUBFUNCTION (RAW, subfunction, CONCAT(SERVICE, _SubfunctionType))
#define SERVICE_ARG(type, name, ...) FIELD_##type(__VA_ARGS__) name
#define SERVICE_ARG_INIT(_, name, ...) m_##name(name)
#define SERVICE_FIELD(type, name, ...) FIELD_##type(__VA_ARGS__) m_##name;
#define SERVICE_GETTER(type, name, ...) \
    FIELD_##type(__VA_ARGS__) get_##name() { return m_##name; };
#define SERVICE_BEGIN                                                  \
    class CONCAT(ServiceResponse_, SERVICE) : public ServiceResponse { \
    public:                                                            \
        CONCAT(ServiceResponse_, SERVICE)                              \
        (MAP_TUPLE_LIST(SERVICE_ARG, RESPONSE_FIELDS))                 \
            : MAP_TUPLE_LIST(SERVICE_ARG_INIT, RESPONSE_FIELDS) {}     \
        ServiceResponseType get_type() {                               \
            return ServiceResponseType::SERVICE;                       \
        }                                                              \
        MAP_TUPLE(SERVICE_GETTER, RESPONSE_FIELDS)                     \
    private:                                                           \
        MAP_TUPLE(SERVICE_FIELD, RESPONSE_FIELDS)                      \
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

#undef FIELD_INT
#undef FIELD_RAW
#undef FIELD_DATA
#undef FIELD_VEC

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
