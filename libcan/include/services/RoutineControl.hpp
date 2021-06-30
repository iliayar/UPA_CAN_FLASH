#pragma once

#include "objects.h"
#include "service.h"
#include "datatypes.h"

namespace Can {

enum class RoutineControlSubfunction {
    StartRoutine = 0x01,
    StopRoutine = 0x02,
    RequestroutineResult = 0x03,
};

namespace ServiceResponse {
class RoutineControl : public ServiceResponse {
public:
    using Subfunction = RoutineControlSubfunction;
    class Builder : public Util::Builder<RoutineControl, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader const& reader) : B() {
            read(reader, object()->m_subfunction);
        }
        auto subfunction(Subfunction value) {
            return field(object()->m_subfunction, value);
        }
        auto routine(std::shared_ptr<Routine> value) {
            return field(object()->m_routine, value);
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
    auto get_routine() { return m_routine.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::RoutineControl;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
    Util::DataField<Routine> m_routine;
};
}  // namespace ServiceResponse

namespace ServiceRequest {

class RoutineControl : public ServiceRequest {
public:
    using Subfunction = RoutineControlSubfunction;
    class Builder : public Util::Builder<RoutineControl, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader const& reader) : B() {
            read(reader, object()->m_subfunction, object()->m_routine);
        }
        auto subfunction(Subfunction value) {
            return field(object()->m_subfunction, value);
        }
        auto routine(std::shared_ptr<Routine> value) {
            return field(object()->m_routine, value);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader const& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_subfunction, m_routine);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_subfunction, m_routine);
    }

    Type get_type() { return m_type.get().value(); }

    auto get_subfunction() { return m_subfunction.get().value(); }
    auto get_routine() { return m_routine.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::RoutineControl;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
    Util::DataField<Routine> m_routine;
};

}  // namespace ServiceRequest

}  // namespace Can
