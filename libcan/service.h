#pragma once

#include "bytes.h"
#include "map.h"

#include <vector>

namespace Can {

enum class ServiceResponseType;
enum class ServiceRequestType;

class ServiceResponse {
public:
    virtual ServiceResponseType get_type() = 0;
};

class ServiceRequest {
public:
    virtual ServiceRequestType get_type() = 0;
    virtual std::vector<uint8_t> dump() = 0;
};

class ServiceResponseFactory {
public:
    ServiceResponseFactory(std::vector<uint8_t>);

    ServiceResponse* get();

private:
#define DATATYPE(...)
#define SUBFUNCTIONS(...)
#define SERVICE_BEGIN ServiceResponse* CONCAT(parse_, SERVICE)();
#include "services/services.h"
#undef SERVICE_BEGIN
#undef SUBFUNCTIONS
#undef DATATYPE
    ServiceResponse* parse_Negative();

    int m_offset;
    int m_size;
    Util::Reader m_reader;
};

ServiceResponseType request_to_response_type(ServiceRequestType);

}  // namespace Can
