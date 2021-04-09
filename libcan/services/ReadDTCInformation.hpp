#pragma once

#include "objects.h"
#include "service.h"
#include "datatypes.h"

namespace Can {

const uint8_t testFailedDTC = 0x01;
const uint8_t confirmedDTC = 0x02;

enum class ReadDTCInformationSubfunction {
    reportNumberOfDTCByStatusMask = 0x01,
    reportDTCByStatusMask = 0x02
};

namespace ServiceResponse {
class ReadDTCInformation : public ServiceResponse {
public:
    using Subfunction = ReadDTCInformationSubfunction;
    class Builder : public Util::Builder<ReadDTCInformation, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            auto& subfunction = object()->m_subfunction;
            subfunction.read(reader);
            if(!subfunction.valid()) {
                fail();
                return;
            }
            auto& status_mask = object()->m_status_mask;
            status_mask.read(reader);
            if(!status_mask.valid()) {
                fail();
                return;
            }
            switch(subfunction.get().value()) {
            case ReadDTCInformationSubfunction::reportDTCByStatusMask: {
                auto& records = object()->m_records;
                while(true) {
                    records.push_back({});
                    auto& field = records.back();
                    field.read(reader);
                    if(!field.valid()) {
                        records.pop_back();
                        break;
                    }
                }
            }
            case ReadDTCInformationSubfunction::reportNumberOfDTCByStatusMask: {
                read(reader, object()->m_format_identifier, object()->m_count);
            }
            default: {
                fail();
                return;
            }
            }
        }
    };

    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        // return Util::dump_args(m_type);
        // TODO
        return {};
    }
    bool write(Util::Writer& writer) {
        // return Util::write_args(writer, m_type);
        // TODO
        return false;
    }

    Type get_type() { return m_type.get().value(); }
    auto get_subfunction() { return m_subfunction.get().value(); }
    auto get_statis_mask() { return m_status_mask.get().value(); }
    auto get_format_identifier() { return m_format_identifier.get().value(); }
    auto get_count() { return m_count.get().value(); }
    auto get_records() {
        std::vector<std::shared_ptr<DTC>> res;
        for(auto& field : m_records) {
            res.push_back(field.get().value());
        }
        return res;
    }
    
private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::ReadDTCInformation;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
    Util::IntField<uint8_t, 8> m_status_mask;
    Util::IntField<uint8_t, 8> m_format_identifier;
    Util::IntField<uint16_t, 16> m_count;
    std::vector<Util::DataField<DTC>> m_records;
};
}  // namespace ServiceResponse

namespace ServiceRequest {

class ReadDTCInformation : public ServiceRequest {
public:
    using Subfunction = ReadDTCInformationSubfunction;
    class Builder : public Util::Builder<ReadDTCInformation, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader& reader) : B() {
            read(reader, object()->m_subfunction, object()->m_mask);
        }
        auto subfunction(Subfunction value) {
            return field(object()->m_subfunction, value);
        }
        auto mask(uint8_t value) {
            return field(object()->m_mask, value);
        }
    };
    static auto build() { return std::make_unique<Builder>(); }
    static auto build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }
    optional<std::vector<uint8_t>> dump() {
        return Util::dump_args(m_type, m_subfunction, m_mask);
    }
    bool write(Util::Writer& writer) {
        return Util::write_args(writer, m_type, m_subfunction, m_mask);
    }

    Type get_type() { return m_type.get().value(); }
    auto get_subfunction() { return m_subfunction.get().value(); }
    auto get_mask() { return m_mask.get().value(); }

private:
    Util::EnumField<Type, uint8_t, 8> m_type = Type::ReadDTCInformation;
    Util::EnumField<Subfunction, uint8_t, 8> m_subfunction;
    Util::IntField<uint8_t, 8> m_mask;
};

}  // namespace ServiceRequest

}  // namespace Can
