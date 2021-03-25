/**
 * @file service_all.h
 * Generates:
 *   - Service request classes with dump methods
 *     Builder classes for requests
 *   - Service response classes with parse methods
 *     in ServiceResponseFactory class.
 * All services defined in {{ services }} folder
 */
#pragma once

#include <vector>

#include "bytes.h"
#include "service.h"

namespace Can {
enum class DataIdentifier { VIN = 0xf190, UPASystemType = 0x200e };

class Data {
public:
    Data(DataIdentifier type, std::vector<uint8_t> value)
        : m_type(type), m_value(value) {}

    DataIdentifier get_type() { return m_type; }
    std::vector<uint8_t> get_value() { return m_value; }

private:
    DataIdentifier m_type;
    std::vector<uint8_t> m_value;
};

class DataFactory {
public:
    DataFactory(std::vector<uint8_t> payload)
        : m_offset(0), m_reader(payload) {}

    DataFactory(Util::Reader reader) : m_offset(0), m_reader(reader) {}

    optional<std::shared_ptr<Data>> get() {
        auto maybe_type = m_reader.read_int<uint16_t>(m_offset, 16);
        if (!maybe_type) {
            return {};
        }
        DataIdentifier type = static_cast<DataIdentifier>(maybe_type.value());
        m_offset += 16;

        switch (type) {
            default:
                return nullptr;
        }
    }

private:
    int m_offset;
    Util::Reader m_reader;
};

enum class CommunicationControlSubfunction {
    disableRxAndTx = 0x03,
};

class CommunicationTypeChanels {
public:
    class Builder {
    public:
        Builder() : m_object(std::make_shared<CommunicationTypeChanels>()) {}
        void normal_communication(int value) {
            m_object->m_normal_communication = value;
        }
        void network_communication(int value) {
            m_object->m_network_communication = value;
        }
        std::shared_ptr<CommunicationTypeChanels> build() { return m_object; }

    private:
        std::shared_ptr<CommunicationTypeChanels> m_object;
    };
    static optional<std::shared_ptr<CommunicationTypeChanels>> parse(
        Util::Reader&);
    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }

private:
    int m_normal_communication;
    int m_network_communication;
};

class CommunicationType {
public:
    class Builder {
    public:
        Builder() : m_object(std::make_shared<CommunicationType>()) {}
        void chanels(std::shared_ptr<CommunicationTypeChanels> value) {
            m_object->chanels = value;
        }
        std::shared_ptr<CommunicationType> build() { return m_object; }

    private:
        std::shared_ptr<CommunicationType> m_object;
    };
    static optional<std::shared_ptr<CommunicationType>> parse(Util::Reader&);
    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }

private:
    int m_unknown1 = 0;
    int m_reserved = 0;
    std::shared_ptr<CommunicationTypeChanels> chanels;
};

namespace ServiceResponse {

class Negative : public ServiceResponse {
public:
    Negative(Type service, uint8_t code) : m_service(service), m_code(code) {}

    Type get_type() { return Type::Negative; }

    Type get_service() { return m_service; }

    uint8_t get_code() { return m_code; }

private:
    Type m_service;
    uint8_t m_code;
};
class ReadDataByIdentifier : public ServiceResponse {};
class WriteDataByIdentifier : public ServiceResponse {};
class SecurityAccess : public ServiceResponse {};
class DiagnosticSessionControl : public ServiceResponse {};
class ControlDTCSettings : public ServiceResponse {};
class CommunicationControl : public ServiceResponse {
public:
    using Subfunction = CommunicationControlSubfunction;
    class Builder {
    public:
        Builder() : m_object(std::make_shared<CommunicationControl>()) {}
        void subfunction(Subfunction value) {
            m_object->m_subfunction = value;
        }
        std::shared_ptr<CommunicationControl> build() {
            return m_object;
        }
    private:
        std::shared_ptr<CommunicationControl> m_object;
    };
    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
private:
    Subfunction m_subfunction;
};
class RequestDownload : public ServiceResponse {};
class TransferData : public ServiceResponse {};
class RequestTransferExit : public ServiceResponse {};
class ECUReset : public ServiceResponse {};
}  // namespace ServiceResponse

namespace ServiceRequest {

class ReadDataByIdentifier : public ServiceRequest {};
class WriteDataByIdentifier : public ServiceRequest {};
class SecurityAccess : public ServiceRequest {};
class DiagnosticSessionControl : public ServiceRequest {};
class ControlDTCSettings : public ServiceRequest {};
class CommunicationControl : public ServiceRequest {
public:
    using Subfunction = CommunicationControlSubfunction;
    class Builder {
    public:
        Builder() : m_object(std::make_shared<CommunicationControl>()) {}
        void subfunction(Subfunction value) {
            m_object->m_subfunction = value;
        }
        void communication_type(std::shared_ptr<CommunicationType> value) {
            m_object->m_communication_type = value;
        }
        std::shared_ptr<CommunicationControl> build() {
            return m_object;
        }
    private:
        std::shared_ptr<CommunicationControl> m_object;
    };
    std::vector<uint8_t> dump();
    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
private:
    Subfunction m_subfunction;
    std::shared_ptr<CommunicationType> m_communication_type;
};
class RequestDownload : public ServiceRequest {};
class TransferData : public ServiceRequest {};
class RequestTransferExit : public ServiceRequest {};
class ECUReset : public ServiceRequest {};
}  // namespace ServiceRequest
}  // namespace Can
