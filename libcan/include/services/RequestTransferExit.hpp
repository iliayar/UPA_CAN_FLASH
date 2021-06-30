#pragma once

#include "objects.h"
#include "service.h"
#include "datatypes.h"

namespace Can {

namespace ServiceResponse {
class RequestTransferExit : public ServiceResponse {
public:
    class Builder : public Util::Builder<RequestTransferExit, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader const& reader) : B() {
            read(reader, object()->m_crc);
        }
        auto crc(uint16_t value) {
            return field(object()->m_crc, value);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader const& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_crc);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_crc);
    }

    Type get_type() { return m_type.get().value(); }
    auto get_crc() { return m_crc.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::RequestTransferExit;
    Util::IntField<uint16_t, 16> m_crc;
};
}  // namespace ServiceResponse

namespace ServiceRequest {

class RequestTransferExit : public ServiceRequest {
public:
    class Builder : public Util::Builder<RequestTransferExit, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader const& reader) : B() {
            read(reader, object()->m_crc);
        }
        auto crc(uint16_t value) {
            return field(object()->m_crc, value);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader const& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_crc);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_crc);
    }

    Type get_type() { return m_type.get().value(); }
    auto get_crc() { return m_crc.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::RequestTransferExit;
    Util::IntField<uint16_t, 16> m_crc;
};

}  // namespace ServiceRequest

}  // namespace Can
