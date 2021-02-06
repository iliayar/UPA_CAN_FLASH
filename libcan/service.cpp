#include "can.h"
#include "service.h"

#include <stdexcept>

Can::DataFactory::DataFactory(std::vector<uint8_t> payload)
    : m_offset(0)
    , m_reader(payload)
{}

Can::DataFactory::DataFactory(Can::Reader reader)
    : m_offset(0)
    , m_reader(reader)
{}

Can::Data* Can::DataFactory::get()
{
    Can::DataIdentifier type = static_cast<Can::DataIdentifier>(m_reader.read_16(m_offset, 16));
    m_offset += 16;

    switch(type) {
    case Can::DataIdentifier::VIN:
        return parse_VIN();
    case Can::DataIdentifier::UPASystemType:
        return parse_UPASystemType();
    default:
        return nullptr;
    }
}

#define PARSE_DATA(name, len) \
Can::Data* Can::DataFactory::parse_##name() \
{ \
    std::vector<uint8_t> data = m_reader.read(m_offset, len*8); \
    m_offset += len*8; \
    m_reader.add_offset(m_offset); \
    return new Can::Data(Can::DataIdentifier::name, data); \
}
PARSE_DATA(VIN, 17)
PARSE_DATA(UPASystemType, 1)
#undef PARSE_DATA


// ---------- Service Request --------------

std::vector<uint8_t> Can::ServiceRequest_ReadDataByIdentifier::dump()
{
    std::vector<uint8_t> payload(3, 0);
    Can::Writer writer(payload);

    int offset = 0;

    writer.write_8(static_cast<uint8_t>(Can::ServiceRequestType::ReadDataByIdentifier), offset, 8);
    offset += 8;
    
    writer.write_16(static_cast<uint16_t>(m_id), offset, 16);
    offset += 16;

    return payload;
}

std::vector<uint8_t> Can::ServiceRequest_WriteDataByIdentifier::dump()
{
    std::vector<uint8_t> payload(1 + 2 + m_data->get_value().size(), 0);
    Can::Writer writer(payload);

    int offset = 0;

    writer.write_8(static_cast<uint8_t>(Can::ServiceRequestType::WriteDataByIdentifier), offset, 8);
    offset += 8;
    
    writer.write_16(static_cast<uint16_t>(m_data->get_type()), offset, 16);
    offset += 16;

    writer.write(m_data->get_value(), offset, m_data->get_value().size()*8);
    offset += m_data->get_value().size()*8;

    return payload;
}


// ---------- Service Response -------------

Can::ServiceResponseFactory::ServiceResponseFactory(std::vector<uint8_t> payload)
    : m_offset(0)
    , m_reader(payload)
{}

Can::ServiceResponse* Can::ServiceResponseFactory::get()
{
    ServiceResponseType type = static_cast<ServiceResponseType>(m_reader.read_8(m_offset, 8));
    m_offset += 8;

    switch(type) {
    case Can::ServiceResponseType::ReadDataByIdentifier:
        return parse_ReadDataByIdentifier();
    case Can::ServiceResponseType::WriteDataByIdentifier:
        return parse_WriteDataByIdentifier();
    default:
        return nullptr;
    }
}

Can::ServiceResponse* Can::ServiceResponseFactory::parse_ReadDataByIdentifier()
{
    m_reader.add_offset(m_offset);
    
    Can::Data* data = Can::DataFactory(m_reader).get();

    return new Can::ServiceResponse_ReadDataByIdentifier(data);
}

Can::ServiceResponse* Can::ServiceResponseFactory::parse_WriteDataByIdentifier()
{
    Can::DataIdentifier id = static_cast<Can::DataIdentifier>(m_reader.read_16(m_offset, 16));

    return new Can::ServiceResponse_WriteDataByIdentifier(id);
}
