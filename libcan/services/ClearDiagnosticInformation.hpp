#pragma once

#include "objects.h"
#include "service.h"
#include "datatypes.h"

namespace Can {

namespace ServiceResponse {
class ClearDiagnostricInformation : public ServiceResponse {
public:
    class Builder : public Util::Builder<ClearDiagnostricInformation, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_group);
        }
        auto group(uint32_t value) {
            return field(object()->m_group, value);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_group);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_group);
    }

    Type get_type() { return m_type.get().value(); }
    auto get_group() { return m_group.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::DiagnosticSessionControl;
    Util::IntField<uint32_t, 24> m_group;
};
}  // namespace ServiceResponse

namespace ServiceRequest {

class ClearDiagnosticInformation : public ServiceRequest {
public:
    class Builder : public Util::Builder<ClearDiagnosticInformation, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {}
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type);
    }

    Type get_type() { return m_type.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::DiagnosticSessionControl;
};

}  // namespace ServiceRequest

}  // namespace Can
