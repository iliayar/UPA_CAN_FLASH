#pragma once

#include "datatypes.h"
#include "objects.h"
#include "service.h"

namespace Can {

enum class SecurityAccessSubfunction {
    requestSeed = 0x03,
    sendKey = 0x04,
};

namespace ServiceResponse {
class SecurityAccess : public ServiceResponse {
public:
    using Subfunction = SecurityAccessSubfunction;
    class Builder : public Util::Builder<SecurityAccess, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader const& reader) : B() {
            auto& subfunction = object()->m_subfunction;
            subfunction.read(reader);
            if(!subfunction.valid()) {
                fail();
                return;
            }
            if(subfunction.get().value() == Subfunction::requestSeed) {
                object()->m_seed.read(reader);
            } 
        }
        auto subfunction(Subfunction value) {
            return field(object()->m_subfunction, value);
        }
        auto seed(uint32_t value) { return field(object()->m_seed, value); }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader const& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_subfunction, m_seed);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_subfunction, m_seed);
    }

    Type get_type() { return m_type.get().value(); }

    auto get_subfunction() { return m_subfunction.get().value(); }
    auto get_seed() { return m_seed.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::SecurityAccess;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
    Util::IntField<uint32_t, 32> m_seed;
};
}  // namespace ServiceResponse

namespace ServiceRequest {

class SecurityAccess : public ServiceRequest {
public:
    using Subfunction = SecurityAccessSubfunction;
    class Builder : public Util::Builder<SecurityAccess, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader const& reader) : B() {
            read(reader, object()->m_subfunction, object()->m_seed_par,
                 object()->m_key);
        }
        auto subfunction(Subfunction value) {
            return field(object()->m_subfunction, value);
        }
        auto seed_par(uint8_t value) {
            return field(object()->m_seed_par, value);
        }
        auto key(uint32_t value) { return field(object()->m_key, value); }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader const& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        if(!m_subfunction.valid()) {
            return {};
        }
        if (m_subfunction.get() == Subfunction::requestSeed) {
            return Util::dump_args(m_type, m_subfunction, m_seed_par);
        } else if (m_subfunction.get() == Subfunction::sendKey) {
            return Util::dump_args(m_type, m_subfunction, m_key);
        }
        return {};
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_subfunction, m_seed_par,
                                m_key);
    }

    Type get_type() { return m_type.get().value(); }

    auto get_subfunction() { return m_subfunction.get().value(); }
    auto get_seed_par() { return m_seed_par.get().value(); }
    auto get_key() { return m_key.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::SecurityAccess;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
    Util::IntField<uint8_t, 8> m_seed_par;
    Util::IntField<uint32_t, 32> m_key;
};

}  // namespace ServiceRequest

}  // namespace Can
