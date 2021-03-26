#pragma once

#include "objects.h"
#include "service.h"
#include "datatypes.h"

namespace Can {

enum class ControlDTCSettingsSubfunction {
    on = 0x01,
    off = 0x02,
};

namespace ServiceResponse {
class ControlDTCSettings : public ServiceResponse {
public:
    using Subfunction = ControlDTCSettingsSubfunction;
    class Builder : public Util::Builder<ControlDTCSettings, Builder> {
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
    Util::EnumField<Type, uint8_t, 8> m_type = Type::ControlDTCSettings;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
};
}  // namespace ServiceResponse

namespace ServiceRequest {

class ControlDTCSettings : public ServiceRequest {
public:
    using Subfunction = ControlDTCSettingsSubfunction;
    class Builder : public Util::Builder<ControlDTCSettings, Builder> {
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
    Util::EnumField<Type, uint8_t, 8> m_type = Type::ControlDTCSettings;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
};

}  // namespace ServiceRequest

}  // namespace Can
