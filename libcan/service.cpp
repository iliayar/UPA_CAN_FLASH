#include "service.h"

#include <memory>
#include <stdexcept>

#include "bytes.h"
#include "service_all.h"

Can::ServiceResponse::Factory::Factory(std::vector<uint8_t> payload)
    : m_reader(payload) {}

optional<std::shared_ptr<Can::ServiceResponse::ServiceResponse>>
Can::ServiceResponse::Factory::get() {
    m_type.read(m_reader);
    if (!m_type.valid()) {
        return {};
    }
    switch (m_type.get().value()) {
        case Can::ServiceResponse::Type::Negative:
            return Can::ServiceResponse::Negative::build(m_reader)->build();
        case Can::ServiceResponse::Type::CommunicationControl:
            return Can::ServiceResponse::CommunicationControl::build(m_reader)
                ->build();
        case Can::ServiceResponse::Type::ControlDTCSettings:
            return Can::ServiceResponse::ControlDTCSettings::build(m_reader)
                ->build();
        case Can::ServiceResponse::Type::DiagnosticSessionControl:
            return Can::ServiceResponse::DiagnosticSessionControl::build(m_reader)
                ->build();
        case Can::ServiceResponse::Type::ECUReset:
            return Can::ServiceResponse::ECUReset::build(m_reader)
                ->build();
        case Can::ServiceResponse::Type::ReadDataByIdentifier:
            return Can::ServiceResponse::ReadDataByIdentifier::build(m_reader)
                ->build();
        case Can::ServiceResponse::Type::RequestDownload:
            return Can::ServiceResponse::RequestDownload::build(m_reader)
                ->build();
        case Can::ServiceResponse::Type::RequestTransferExit:
            return Can::ServiceResponse::RequestTransferExit::build(m_reader)
                ->build();
        case Can::ServiceResponse::Type::SecurityAccess:
            return Can::ServiceResponse::SecurityAccess::build(m_reader)
                ->build();
        case Can::ServiceResponse::Type::TransferData:
            return Can::ServiceResponse::TransferData::build(m_reader)
                ->build();
        case Can::ServiceResponse::Type::WriteDataByIdentifier:
            return Can::ServiceResponse::WriteDataByIdentifier::build(m_reader)
                ->build();
        case Can::ServiceResponse::Type::ReadDTCInformation:
            return Can::ServiceResponse::ReadDTCInformation::build(m_reader)
                ->build();
        case Can::ServiceResponse::Type::ClearDiagnosticInformation:
            return Can::ServiceResponse::ClearDiagnostricInformation::build(m_reader)
                ->build();
        case Can::ServiceResponse::Type::RoutineControl:
            return Can::ServiceResponse::RoutineControl::build(m_reader)
                ->build();
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
