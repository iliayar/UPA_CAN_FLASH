#pragma once

#include "objects.h"
#include "service.h"
#include "datatypes.h"

namespace Can {

namespace ServiceResponse {
class WriteDataByIdentifier : public ServiceResponse {
public:
    class Builder : public Util::Builder<WriteDataByIdentifier, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_id);
        }
        auto data(DataIdentifier value) {
            return field(object()->m_id, value);
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
        return Util::dump_args(m_type, m_id);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_id);
    }

    Type get_type() { return m_type.get().value(); }
    auto get_id() { return m_id.get().value(); }
    
private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::WriteDataByIdentifier;
    Util::EnumField<DataIdentifier, uint16_t, 16> m_id;
};
}  // namespace ServiceResponse

namespace ServiceRequest {

class WriteDataByIdentifier : public ServiceRequest {
public:
    class Builder : public Util::Builder<WriteDataByIdentifier, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_data);
        }
        auto id(std::shared_ptr<Data> value) {
            return field(object()->m_data, value);
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
        return Util::dump_args(m_type, m_data);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_data);
    }

    auto get_data() { return m_data.get().value(); }
    Type get_type() { return m_type.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::WriteDataByIdentifier;
    Util::DataField<Data> m_data;
};

}  // namespace ServiceRequest

}  // namespace Can
