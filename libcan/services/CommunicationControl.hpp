#pragma once

#include "objects.h"
#include "service.h"
#include "datatypes.h"

namespace Can {

enum class CommunicationControlSubfunction {
    disableRxAndTx = 0x03,
};

namespace ServiceResponse {
class CommunicationControl : public ServiceResponse {
public:
    using Subfunction = CommunicationControlSubfunction;
    class Builder : public Util::Builder<CommunicationControl, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_subfunction);
        }
        auto subfunction(Subfunction value) {
            return field(object()->m_subfunction, value);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader& reader) {
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
    Util::EnumField<Type, uint8_t, 8> m_type = Type::CommunicationControl;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
};
}  // namespace ServiceResponse

namespace ServiceRequest {

class CommunicationControl : public ServiceRequest {
public:
    using Subfunction = CommunicationControlSubfunction;
    class Builder : public Util::Builder<CommunicationControl, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_subfunction, object()->m_communication_type);
        }
        auto subfunction(Subfunction value) {
            return field(object()->m_subfunction, value);
        }
        auto communication_type(std::shared_ptr<CommunicationType> value) {
            return field(object()->m_communication_type, value);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_subfunction, m_communication_type);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_subfunction, m_communication_type);
    }

    Type get_type() { return m_type.get().value(); }

    auto get_subfunction() { return m_subfunction.get().value(); }
    auto get_communication_type() { return m_communication_type.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::CommunicationControl;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
    Util::DataField<CommunicationType> m_communication_type;
};

}  // namespace ServiceRequest

}  // namespace Can
