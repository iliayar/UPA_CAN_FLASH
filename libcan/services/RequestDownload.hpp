#pragma once

#include "datatypes.h"
#include "objects.h"
#include "service.h"

namespace Can {

namespace ServiceResponse {
class RequestDownload : public ServiceResponse {
public:
    class Builder : public Util::Builder<RequestDownload, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            object()->m_length_format.read(reader);
            if (!object()->m_length_format.valid()) {
                fail();
                return;
            }
            object()->m_max_blocks_number.resize(
                object()->m_length_format.get().value()->get_memory_size() * 8);
            object()->m_max_blocks_number.read(reader);
            if (!object()->m_max_blocks_number.valid()) {
                fail();
                return;
            }
        }

        auto max_blocks_number(std::vector<uint8_t> value) {
            return field(object()->m_max_blocks_number, value);
        }
        auto length_format(std::shared_ptr<LengthFormatIdentifier> value) {
            object()->m_max_blocks_number.resize(value->get_memory_size() * 8);
            return field(object()->m_length_format, value);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_length_format, m_max_blocks_number);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_length_format,
                                m_max_blocks_number);
    }

    Type get_type() { return m_type.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::RequestDownload;
    Util::DataField<LengthFormatIdentifier> m_length_format;
    Util::VarVecField m_max_blocks_number;
};
}  // namespace ServiceResponse

namespace ServiceRequest {

class RequestDownload : public ServiceRequest {
public:
    class Builder : public Util::Builder<RequestDownload, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            object()->m_data_format.read(reader);
            if (!object()->m_data_format.valid()) {
                fail();
                return;
            }
            object()->m_address_len_format.read(reader);
            if (!object()->m_address_len_format.valid()) {
                fail();
                return;
            }
            object()->m_memory_addr.resize(object()
                                               ->m_address_len_format.get()
                                               .value()
                                               ->get_memory_address() *
                                           8);
            object()->m_memory_size.resize(object()
                                               ->m_address_len_format.get()
                                               .value()
                                               ->get_memory_size() *
                                           8);
        }

        auto data_format(std::shared_ptr<DataFormatIdentifier> value) {
            return field(object()->m_data_format, value);
        }
        auto address_len_format(
            std::shared_ptr<DataAndLengthFormatIdentifier> value) {
            object()->m_memory_addr.resize(value->get_memory_address() * 8);
            object()->m_memory_size.resize(value->get_memory_size() * 8);
            return field(object()->m_address_len_format, value);
        }
        auto memory_addr(std::vector<uint8_t> value) {
            return field(object()->m_memory_addr, value);
        }
        auto memory_size(std::vector<uint8_t> value) {
            return field(object()->m_memory_size, value);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() { return Util::dump_args(m_type); }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type);
    }

    Type get_type() { return m_type.get().value(); }

    auto get_data_format() { return m_data_format.get().value(); }

    auto get_address_len_format() { return m_address_len_format.get().value(); }

    auto get_memory_addr() { return m_memory_addr.get().value(); }

    auto get_memory_size() { return m_memory_size.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::RequestDownload;
    Util::DataField<DataFormatIdentifier> m_data_format;
    Util::DataField<DataAndLengthFormatIdentifier> m_address_len_format;
    Util::VarVecField m_memory_addr;
    Util::VarVecField m_memory_size;
};

}  // namespace ServiceRequest

}  // namespace Can
