#pragma once

#include "objects.h"
#include "service.h"
#include "datatypes.h"

namespace Can {

enum class DiagnosticSessionControlSubfunction {
    extendDiagnosticSession = 0x03,
    programmingSession = 0x02,
};

namespace ServiceResponse {
class DiagnosticSessionControl : public ServiceResponse {
public:
    using Subfunction = DiagnosticSessionControlSubfunction;
    class Builder : public Util::Builder<DiagnosticSessionControl, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader const& reader) : B() {
            read(reader, object()->m_subfunction);
        }
        auto subfunction(Subfunction value) {
            return field(object()->m_subfunction, value);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader const& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_subfunction);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_subfunction);
    }

    Type get_type() { return m_type.get().value(); }

    auto get_subfunction() { return m_subfunction.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::DiagnosticSessionControl;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
};
}  // namespace ServiceResponse

namespace ServiceRequest {

class DiagnosticSessionControl : public ServiceRequest {
public:
    using Subfunction = DiagnosticSessionControlSubfunction;
    class Builder : public Util::Builder<DiagnosticSessionControl, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader const& reader) : B() {
            read(reader, object()->m_subfunction);
        }
        auto subfunction(Subfunction value) {
            return field(object()->m_subfunction, value);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader const& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_subfunction);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_subfunction);
    }

    Type get_type() { return m_type.get().value(); }

    auto get_subfunction() { return m_subfunction.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::DiagnosticSessionControl;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
};

}  // namespace ServiceRequest

}  // namespace Can
