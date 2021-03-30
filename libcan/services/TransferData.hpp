#pragma once

#include "objects.h"
#include "service.h"
#include "datatypes.h"

namespace Can {

namespace ServiceResponse {
class TransferData : public ServiceResponse {
public:
    class Builder : public Util::Builder<TransferData, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_block_counter, object()->m_data);
        }
        auto block_counter(uint16_t value) {
            return field(object()->m_block_counter, value);
        }
        auto data(std::vector<uint8_t> data) {
            return field(object()->m_data, data);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_block_counter);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_block_counter);
    }

    Type get_type() { return m_type.get().value(); }
    auto get_block_counter() { return m_block_counter.get().value(); }
    auto get_data() { return m_data.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::TransferData;
    Util::IntField<uint16_t, 16> m_block_counter;
    Util::VarVecField m_data;
};
}  // namespace ServiceResponse

namespace ServiceRequest {

class TransferData : public ServiceRequest {
public:
    class Builder : public Util::Builder<TransferData, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_block_counter, object()->m_data);
        }
        auto block_counter(uint16_t value) {
            return field(object()->m_block_counter, value);
        }
        auto data(std::vector<uint8_t> value) {
            object()->m_data.resize(value.size()*8);
            return field(object()->m_data, value);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_block_counter, m_data);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_block_counter, m_data);
    }

    Type get_type() { return m_type.get().value(); }
    auto get_block_counter() { return m_block_counter.get().value(); }
    auto get_data() { return m_data.get().value(); }
    
private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::TransferData;
    Util::IntField<uint16_t, 16> m_block_counter;
    Util::VarVecField m_data;
};

}  // namespace ServiceRequest

}  // namespace Can
