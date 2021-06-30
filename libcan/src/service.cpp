#include "service.h"

#include <memory>
#include <stdexcept>

#include "bytes.h"
#include "service_all.h"

Can::ServiceResponse::Factory::Factory(std::vector<uint8_t> const& payload)
    : m_reader(payload) {}

optional<std::shared_ptr<Can::ServiceResponse::ServiceResponse>>
Can::ServiceResponse::Factory::get() {
    using namespace Can::ServiceResponse;

    m_type.read(m_reader);
    if (!m_type.valid()) {
        return {};
    }

    switch (m_type.get().value()) {
        case Type::Negative:
            return Negative::build(m_reader)->build();
        case Type::CommunicationControl:
            return CommunicationControl::build(m_reader)->build();
        case Type::ControlDTCSettings:
            return ControlDTCSettings::build(m_reader)->build();
        case Type::DiagnosticSessionControl:
            return DiagnosticSessionControl::build(m_reader)->build();
        case Type::ECUReset:
            return ECUReset::build(m_reader)->build();
        case Type::ReadDataByIdentifier:
            return ReadDataByIdentifier::build(m_reader)->build();
        case Type::RequestDownload:
            return RequestDownload::build(m_reader)->build();
        case Type::RequestTransferExit:
            return RequestTransferExit::build(m_reader)->build();
        case Type::SecurityAccess:
            return SecurityAccess::build(m_reader)->build();
        case Type::TransferData:
            return TransferData::build(m_reader)->build();
        case Type::WriteDataByIdentifier:
            return WriteDataByIdentifier::build(m_reader)->build();
        case Type::ReadDTCInformation:
            return ReadDTCInformation::build(m_reader)->build();
        case Type::ClearDiagnosticInformation:
            return ClearDiagnostricInformation::build(m_reader)->build();
        case Type::RoutineControl:
            return RoutineControl::build(m_reader)->build();
        default:
            return {};
    }
}

/* FIXME */
Can::ServiceResponse::Type Can::request_to_response_type(
    Can::ServiceRequest::Type type) {
    switch (type) {
        case Can::ServiceRequest::Type::CommunicationControl:
            return Can::ServiceResponse::Type::CommunicationControl;
        case Can::ServiceRequest::Type::ControlDTCSettings:
            return Can::ServiceResponse::Type::ControlDTCSettings;
        case Can::ServiceRequest::Type::DiagnosticSessionControl:
            return Can::ServiceResponse::Type::DiagnosticSessionControl;
        case Can::ServiceRequest::Type::ECUReset:
            return Can::ServiceResponse::Type::ECUReset;
        case Can::ServiceRequest::Type::ReadDataByIdentifier:
            return Can::ServiceResponse::Type::ReadDataByIdentifier;
        case Can::ServiceRequest::Type::RequestDownload:
            return Can::ServiceResponse::Type::RequestDownload;
        case Can::ServiceRequest::Type::RequestTransferExit:
            return Can::ServiceResponse::Type::RequestTransferExit;
        case Can::ServiceRequest::Type::SecurityAccess:
            return Can::ServiceResponse::Type::SecurityAccess;
        case Can::ServiceRequest::Type::TransferData:
            return Can::ServiceResponse::Type::TransferData;
        case Can::ServiceRequest::Type::WriteDataByIdentifier:
            return Can::ServiceResponse::Type::WriteDataByIdentifier;
        case Can::ServiceRequest::Type::ReadDTCInformation:
            return Can::ServiceResponse::Type::ReadDTCInformation;
        case Can::ServiceRequest::Type::ClearDiagnosticInformation:
            return Can::ServiceResponse::Type::ClearDiagnosticInformation;
        case Can::ServiceRequest::Type::RoutineControl:
            return Can::ServiceResponse::Type::RoutineControl;
        default:
            return Can::ServiceResponse::Type::Negative;
    }
}
