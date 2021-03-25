#pragma once

#include "objects.h"
#include "service.h"
#include "datatypes.h"

namespace Can {

enum class ECUResetSubfuntion {
    hardReset = 0x01,
    keyOffOnReset = 0x02,
    softReset = 0x03,
    enableRapidPowerShutDown = 0x04,
    disableRapidPowerShutDown = 0x05
};

namespace ServiceResponse {
class ECUReset : public ServiceResponse {
public:
    using Subfunction = ECUResetSubfuntion;
    class Builder : public Util::Builder<ECUReset, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_subfunction);
        }
        auto subfunction(Subfunction value) {
            return field(object()->m_subfunction, value);
        }

    protected:
        std::unique_ptr<Builder> self() {
            return std::unique_ptr<Builder>(this);
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
    Util::EnumField<Type, uint8_t, 8> m_type = Type::ECUReset;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
};
}  // namespace ServiceResponse

namespace ServiceRequest {

class ECUReset : public ServiceRequest {
public:
    using Subfunction = ECUResetSubfuntion;
    class Builder : public Util::Builder<ECUReset, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_subfunction, object()->m_power_down_timer);
        }
        auto subfunction(Subfunction value) {
            return field(object()->m_subfunction, value);
        }
        auto power_down_timer(uint8_t value) {
            return field(object()->m_power_down_timer, value);
        }

    protected:
        std::unique_ptr<Builder> self() {
            return std::unique_ptr<Builder>(this);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_subfunction, m_power_down_timer);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_subfunction, m_power_down_timer);
    }

    Type get_type() { return m_type.get().value(); }

    auto get_subfunction() { return m_subfunction.get().value(); }
    auto get_power_down_timer() { return m_power_down_timer.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::ECUReset;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
    Util::IntField<uint8_t, 8> m_power_down_timer;
};

}  // namespace ServiceRequest

}  // namespace Can
