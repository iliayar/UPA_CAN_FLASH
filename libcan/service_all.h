/**
 * @file service_all.h
 * Generates:
 *   - Service request classes with dump methods
 *     Builder classes for requests
 *   - Service response classes with parse methods
 *     in ServiceResponseFactory class.
 * All services defined in {{ services }} folder
 */
#pragma once

#include <vector>

#include "bytes.h"
#include "service.h"
#include "objects.h"

#include "services/CommunicationControl.hpp"
#include "services/ECUReset.hpp"
#include "services/DiagnosticSessionControl.hpp"
#include "services/ControlDTCSettings.hpp"
#include "services/ReadDataByIdentifier.hpp"
#include "services/WriteDataByIdentifier.hpp"
#include "services/SecurityAccess.hpp"
#include "services/RequestDownload.hpp"
#include "services/TransferData.hpp"
#include "services/RequestTransferExit.hpp"
#include "services/ReadDTCInformation.hpp"
#include "services/ClearDiagnosticInformation.hpp"
#include "services/RoutineControl.hpp"

namespace Can {
namespace ServiceResponse {

class Negative : public ServiceResponse {
public:
    class Builder : public Util::Builder<Negative, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_serice, object()->m_code);
        }
        auto service(Can::ServiceRequest::Type value) {
            return field(object()->m_serice, value);
        }

        auto code(uint8_t value) {
            return field(object()->m_code, value);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_serice, m_code);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_serice, m_code);
    }

    Type get_type() { return m_type.get().value(); }
    auto get_service() { return m_serice.get().value(); }
    auto get_code() { return m_code.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::Negative;
    Util::EnumField<Can::ServiceRequest::Type, uint8_t, 8> m_serice;
    Util::IntField<uint8_t, 8> m_code;
};
}  // namespace ServiceResponse
}  // namespace Can
