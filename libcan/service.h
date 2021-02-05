#pragma once

#include "can.h"

#include <vector>

namespace Can {

    enum class DataIdentifier {
        VIN = 0xf190,
        UPASystemType = 0x200e
    };

    class Data {
    public:
        Data(DataIdentifier type, std::vector<uint8_t> value)
            : m_type(type)
            , m_value(value)
            {}
        
        DataIdentifier get_type() { return m_type; }
        std::vector<uint8_t> get_value() { return m_value; }
    private:
        DataIdentifier m_type;
        std::vector<uint8_t> m_value;
    };

    class DataFactory {
    public:
        DataFactory(std::vector<uint8_t>);
        DataFactory(Reader);

        Data* get();
    private:
        Data* parse_VIN();
        Data* parse_UPASystemType();
        
        int m_offset;
        Reader m_reader;
    };

        
    

// --------------- Service Request --------------------

    enum class ServiceRequestType {
        ReadDataByIdentifier = 0x22,
        WriteDataByIdentifier = 0x2e
    };

    class ServiceRequest {
    public:
        virtual ServiceRequestType get_type() = 0;
        virtual std::vector<uint8_t> dump() = 0;
    };

    class ServiceRequest_ReadDataByIdentifier : public ServiceRequest {
    public:
        ServiceRequest_ReadDataByIdentifier(DataIdentifier id)
            : m_id(id)
            {}

        ServiceRequestType get_type() { return ServiceRequestType::ReadDataByIdentifier; }
        std::vector<uint8_t> dump();
    private:
        DataIdentifier m_id;
    };

    class ServiceRequest_WriteDataByIdentifier : public ServiceRequest {
    public:
        ServiceRequest_WriteDataByIdentifier(Data data)
            : m_data(data)
            {}

        ServiceRequestType get_type() { return ServiceRequestType::WriteDataByIdentifier; }
        std::vector<uint8_t> dump();
    private:
        Data m_data;
    };

// --------------- Service Response -------------------
    
    enum class ServiceResponseType {
        ReadDataByIdentifier = 0x62,
        WriteDataByIdentifier = 0x6e
    };


    class ServiceResponse {
    public:
        virtual ServiceResponseType get_type() = 0;
    };

    class ServiceResponse_ReadDataByIdentifier : public ServiceResponse {
    public:
        ServiceResponse_ReadDataByIdentifier(Data* data)
            : m_data(data)
            {}
        
        ServiceResponseType get_type() { return ServiceResponseType::ReadDataByIdentifier; }
        Data* get_data() { return m_data; }
        
    private:
        Data* m_data;
    };

    class ServiceResponse_WriteDataByIdentifier : public ServiceResponse {
    public:
        ServiceResponse_WriteDataByIdentifier(DataIdentifier id)
            : m_id(id)
            {}
        
        ServiceResponseType get_type() { return ServiceResponseType::WriteDataByIdentifier; }
        DataIdentifier get_id() { return m_id; }
        
    private:
        DataIdentifier m_id;
    };
    

    class ServiceResponseFactory {
    public:
        ServiceResponseFactory(std::vector<uint8_t>);

        ServiceResponse* get();
    private:
        ServiceResponse* parse_ReadDataByIdentifier();
        ServiceResponse* parse_WriteDataByIdentifier();
        
        int m_offset;
        Reader m_reader;
    };
    
}
