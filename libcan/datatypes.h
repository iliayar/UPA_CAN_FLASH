/**
 * @file datatypes.h Contains heplper objects. These object are
 * implemented using the Builder pattern with the same interface as request/response, frames
 */
#pragma once

#include "objects.h"
#include <map>

namespace Can {
    enum class DataIdentifier { VIN = 0xf190, UPASystemType = 0x200e, Conf = 0x2044, THRFOUPA = 0xb001 };

class DTC {
public:
    class Builder : public Util::Builder<DTC, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_type, object()->m_status, object()->m_low);
        }
        auto type(uint16_t value) {
            return field(object()->m_type, value);
        }
        auto low(uint8_t value) {
            return field(object()->m_low, value);
        }

        auto status(uint8_t value) {
            return field(object()->m_status, value);
        }
    protected:
        std::unique_ptr<Builder> self() {
            return std::unique_ptr<Builder>(this);
        }
    };
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_status);
    }
    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }

    auto get_type() { return m_type.get().value(); }
    auto get_status() { return m_status.get().value(); }
    auto get_low() { return m_low.get().value(); }
private:
    Util::IntField<uint16_t, 16> m_type;
    Util::IntField<uint8_t, 8> m_low;
    Util::IntField<uint8_t, 8> m_status;
};
    
class Data {
public:

    static std::map<uint16_t, int> m_sizes;
    
    class Builder : public Util::Builder<Data, Builder> {
    protected:
        std::unique_ptr<Builder> self() {
            return std::unique_ptr<Builder>(this);
        }

    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            auto& type = this->object()->m_type;
            type.read(reader);
            if(!type.valid()) {
                fail();
                return;
            }
            auto& value = this->object()->m_value;

            auto i = m_sizes.find(type.get().value());
            if(i == m_sizes.end()) {
                fail();
                return;
            }
            value.resize(i->second);
            value.read(reader);
            if(!value.valid()) {
                fail();
            }
        }
        auto value(std::vector<uint8_t> value) {
            return this->field(object()->m_value, value);
        }
        auto type(DataIdentifier value) {
            return this->field(object()->m_type, static_cast<uint16_t>(value));
        }
        auto raw_type(uint16_t value) {
            return this->field(object()->m_type, value);
        }
    };
    DataIdentifier get_type() { return static_cast<DataIdentifier>(m_type.get().value()); }
    auto get_value() { return m_value.get().value(); }
    uint16_t get_raw_type() { return m_type.get().value(); }

    optional<std::vector<uint8_t>> dump() { return Util::dump_args(m_type, m_value); }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_value);
    }

    static auto build() {
        return std::make_unique<Builder>();
    }
    static auto build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }

    static void register_did(uint16_t did, int size) {
        if(m_sizes.find(did) == m_sizes.end()) {
            m_sizes[did] = size;
        }
    }

private:
    Util::IntField<uint16_t, 16> m_type;
    Util::VarVecField m_value;
};

class CommunicationTypeChanels {
public:
    class Builder;
    class Builder : public Util::Builder<CommunicationTypeChanels, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_normal_communication,
                 object()->m_network_communication);
        }
        auto normal_communication(int value) {
            return field(object()->m_normal_communication, value);
        }
        auto network_communication(int value) {
            return field(object()->m_network_communication, value);
        }
    protected:
        std::unique_ptr<Builder> self() {
            return std::unique_ptr<Builder>(this);
        }
    };
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_normal_communication, m_network_communication);
    }
    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }

    auto get_normal_communication() {
        return m_normal_communication.get().value();
    }

    auto get_network_communication() {
        return m_network_communication.get().value();
    }
    
private:
    Util::IntField<uint8_t, 1> m_normal_communication;
    Util::IntField<uint8_t, 1> m_network_communication;
};

class CommunicationType {
public:
    class Builder;
    class Builder : public Util::Builder<CommunicationType, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_smth, object()->m_reserved, object()->m_chanels);
        }
        auto chanels(std::shared_ptr<CommunicationTypeChanels> chanels) {
            return field(object()->m_chanels, chanels);
        }
    protected:
        std::unique_ptr<Builder> self() {
            return std::unique_ptr<Builder>(this);
        }
    };
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_smth, m_reserved, m_chanels);
    }
    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }

    auto get_chanels() {
        return m_chanels.get().value();
    }

private:
    Util::IntField<uint8_t, 4> m_smth = 0;
    Util::IntField<uint8_t, 2> m_reserved = 0;
    Util::DataField<CommunicationTypeChanels> m_chanels;
};

class DataFormatIdentifier {
public:
    class Builder;
    class Builder : public Util::Builder<DataFormatIdentifier, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_compression_method, object()->m_encrypting_method);
        }
        auto compression_method(uint8_t value) {
            return field(object()->m_compression_method, value);
        }
        auto encryting_method(uint8_t value) {
            return field(object()->m_encrypting_method, value);
        }
    protected:
        std::unique_ptr<Builder> self() {
            return std::unique_ptr<Builder>(this);
        }
    };
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_compression_method, m_encrypting_method);
    }
    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }

    auto get_compression_method() { return m_compression_method.get().value(); }
    auto get_encrypting_method() { return m_encrypting_method.get().value(); }

private:
    Util::IntField<uint8_t, 4> m_compression_method;
    Util::IntField<uint8_t, 4> m_encrypting_method;
};

class DataAndLengthFormatIdentifier {
public:
    class Builder;
    class Builder : public Util::Builder<DataAndLengthFormatIdentifier, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_memory_size, object()->m_memory_address);
        }
        auto memory_size(uint8_t value) {
            return field(object()->m_memory_size, value);
        }
        auto memory_address(uint8_t value) {
            return field(object()->m_memory_address, value);
        }
    protected:
        std::unique_ptr<Builder> self() {
            return std::unique_ptr<Builder>(this);
        }
    };
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_memory_size, m_memory_address);
    }
    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }

    auto get_memory_size() { return m_memory_size.get().value(); }
    auto get_memory_address() { return m_memory_address.get().value(); }

private:
    Util::IntField<uint8_t, 4> m_memory_size;
    Util::IntField<uint8_t, 4> m_memory_address;
};

class LengthFormatIdentifier {
public:
    class Builder;
    class Builder : public Util::Builder<LengthFormatIdentifier, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_memory_size, object()->m_reserved);
        }
        auto memory_size(uint8_t value) {
            return field(object()->m_memory_size, value);
        }
    protected:
        std::unique_ptr<Builder> self() {
            return std::unique_ptr<Builder>(this);
        }
    };
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_memory_size, m_reserved);
    }
    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }

    auto get_memory_size() { return m_memory_size.get().value(); }

private:
    Util::IntField<uint8_t, 4> m_memory_size;
    Util::IntField<uint8_t, 4> m_reserved;
};

class Routine {
public:
    class Builder;
    class Builder : public Util::Builder<Routine, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            object()->m_data.all();
            read(reader, object()->m_id, object()->m_data);
        }
        auto id(uint16_t value) {
            return field(object()->m_id, value);
        }
        auto data(std::vector<uint8_t> value) {
            return field(object()->m_data, value);
        }
    protected:
        std::unique_ptr<Builder> self() {
            return std::unique_ptr<Builder>(this);
        }
    };
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_id, m_data);
    }
    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }

    auto get_id() { return m_id.get().value(); }
    auto get_data() { return m_data.get().value(); }

private:
    Util::IntField<uint16_t, 16> m_id;
    Util::VarVecField m_data;
};
}  // namespace Can
