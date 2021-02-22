/**
 * @file service.h
 * Provide abstract calss of service request and response
 * All service type are defined in {{ service_all.h }}
 */
#pragma once

#include <memory>
#include <vector>

#include "bytes.h"
#include "map.h"

namespace Can {

enum class ServiceResponseType;
enum class ServiceRequestType;

/**
 * Service response class
 */
class ServiceResponse {
public:
    /**
     * @return enum service response type
     */
    virtual ServiceResponseType get_type() = 0;
};

/**
 * Service request class
 */
class ServiceRequest {
public:
    /**
     * @return enum service request type
     */
    virtual ServiceRequestType get_type() = 0;

    /**
     * Convert request to raw bytes
     * @return bytes representation of request
     */
    virtual std::vector<uint8_t> dump() = 0;
};

class ServiceResponseFactory {
public:
    /**
     * @param binary representation of service response
     */
    ServiceResponseFactory(std::vector<uint8_t>);

    /**
     * @return Service response parsed from passed bytes
     * Returns null if could not parse service response
     */
    std::shared_ptr<ServiceResponse> get();

private:
#define DATATYPE(...)
#define SUBFUNCTIONS(...)
#define SERVICE_BEGIN \
    std::shared_ptr<ServiceResponse> CONCAT(parse_, SERVICE)();
#include "services/services.h"
#undef SERVICE_BEGIN
#undef SUBFUNCTIONS
#undef DATATYPE
    std::shared_ptr<ServiceResponse> parse_Negative();

    int m_offset;
    int m_size;
    Util::Reader m_reader;
};

ServiceResponseType request_to_response_type(ServiceRequestType);

}  // namespace Can
