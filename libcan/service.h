/**
 * @file service.h
 * Provide abstract calss of service request and response
 * All service type are defined in {{ service_all.h }}
 */
#pragma once

#include <memory>
#include <vector>

#include "bytes.h"
#include "objects.h"

namespace Can {

namespace ServiceResponse {

enum class Type {
    ReadDataByIdentifier = 0x62,
    WriteDataByIdentifier = 0x6e,
    SecurityAccess = 0x67,
    DiagnosticSessionControl = 0x50,
    ControlDTCSettings = 0xc5,
    CommunicationControl = 0x68,
    RequestDownload = 0x74,
    TransferData = 0x76,
    RequestTransferExit = 0x77,
    ECUReset = 0x51,
    Negative = 0x7f,
    ReadDTCInformation = 0x59,
    ClearDiagnosticInformation = 0x54,
};
/**
 * Service response class
 */
class ServiceResponse {
public:
    /**
     * @return enum service response type
     */
    virtual Type get_type() = 0;
};
class Factory {
public:
    /**
     * @param binary representation of service response
     */
    Factory(std::vector<uint8_t>);

    /**
     * @return Service response parsed from passed bytes
     * Returns null if could not parse service response
     */
    optional<std::shared_ptr<ServiceResponse>> get();

private:
    Util::EnumField<Type, uint8_t, 8> m_type;
    Util::Reader m_reader;
};

}  // namespace ServiceResponse

namespace ServiceRequest {
enum class Type {
    ReadDataByIdentifier = 0x22,
    WriteDataByIdentifier = 0x2e,
    SecurityAccess = 0x27,
    DiagnosticSessionControl = 0x10,
    ControlDTCSettings = 0x85,
    CommunicationControl = 0x28,
    RequestDownload = 0x34,
    TransferData = 0x36,
    RequestTransferExit = 0x37,
    ECUReset = 0x11,
    ReadDTCInformation = 0x19,
    ClearDiagnosticInformation = 0x14,
};
/**
 * Service request class
 */
class ServiceRequest {
public:
    /**
     * @return enum service request type
     */
    virtual Type get_type() = 0;

    /**
     * Convert request to raw bytes
     * @return bytes representation of request
     */
    virtual optional<std::vector<uint8_t>> dump() = 0;
};
}  // namespace ServiceRequest

ServiceResponse::Type request_to_response_type(ServiceRequest::Type);

}  // namespace Can
