#include "task.h"

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "service_all.h"
#include "util.h"

using namespace Can;

std::shared_ptr<Can::ServiceResponse> Can::AsyncTask::call_imp(
    std::shared_ptr<Can::ServiceRequest> request) {
    DEBUG(info, "task");
    while (true) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_request == nullptr) {
                m_request = request;
                m_wait_response = true;
                DEBUG(info, "task pushed request");
                break;
            }
        }
    }
    DEBUG(info, "task waiting response");
    while (true) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_response != nullptr) {
                DEBUG(info, "task get response");
                std::shared_ptr<Can::ServiceResponse> response = m_response;
                if (response->get_type() ==
                    Can::ServiceResponseType::Negative) {
                    if (static_cast<Can::ServiceResponse_Negative*>(
                            response.get())
                            ->get_service() != request->get_type()) {
                        m_response = nullptr;
                        DEBUG(info, "task response invalid error service code");
                        m_logger->warning("Invalid error service code");
                        continue;
                    }
                    if (static_cast<Can::ServiceResponse_Negative*>(
                            response.get())
                            ->get_code() == 0x78) {
                        m_response = nullptr;
                        DEBUG(info, "task response error service code = 0x78");
                        m_logger->warning("Waiting for positive resposnse");
                        continue;
                    }
                } else if (response->get_type() !=
                           Can::request_to_response_type(request->get_type())) {
                    m_response = nullptr;
                    DEBUG(info, "task response code wrong");
                    m_logger->warning("Invalid response code");
                    continue;
                }
                m_response = nullptr;
                m_wait_response = false;
                DEBUG(info, "task fetching response");
                return response;
            }
        }
    }
}

std::shared_ptr<ServiceRequest> ReadWriteTask::fetch_request() {
    m_step++;
    switch (m_step - 1) {
        case 0:
            return ServiceRequest_ReadDataByIdentifier::build()
                ->id(DataIdentifier::UPASystemType)
                ->build();
        case 2:
            return ServiceRequest_WriteDataByIdentifier::build()
                ->data(new Data(DataIdentifier::VIN,
                                ::Util::str_to_vec("HELLO ANYBODY ...")))
                ->build();
        case 4:
            return ServiceRequest_ReadDataByIdentifier::build()
                ->id(DataIdentifier::VIN)
                ->build();
        default:
            m_step--;
            return nullptr;
    }
}

void ReadWriteTask::push_response(std::shared_ptr<ServiceResponse> response) {
    m_step++;
    switch (m_step - 1) {
        case 1: {
            uint8_t type = static_cast<ServiceResponse_ReadDataByIdentifier*>(
                               response.get())
                               ->get_data()
                               ->get_value()[0];
            std::cout << "UPASystemType = " << (int)type << std::endl;
            break;
        }
        case 3: {
            break;
        }
        case 5: {
            std::string VIN = ::Util::vec_to_str(
                static_cast<ServiceResponse_ReadDataByIdentifier*>(
                    response.get())
                    ->get_data()
                    ->get_value());
            std::cout << "VIN = " << VIN << std::endl;
            break;
        }
        default:
            m_step--;
            return;
    }
}

void ReadWriteThreadedTask::task() {
    std::shared_ptr<ServiceResponse> response;
    m_logger->info("Reading UPASystemType");
    response = call(ServiceRequest_ReadDataByIdentifier::build()
                        ->id(DataIdentifier::UPASystemType)
                        ->build());
    uint8_t type =
        static_cast<ServiceResponse_ReadDataByIdentifier*>(response.get())
            ->get_data()
            ->get_value()[0];
    std::stringstream ss;
    ss << "UPASystemType = " << std::hex << (int)type;
    m_logger->info(ss.str());

    m_logger->info("Writing VIN");
    call(ServiceRequest_WriteDataByIdentifier::build()
             ->data(new Data(DataIdentifier::VIN,
                             ::Util::str_to_vec("HELLO ANYBODY ...")))
             ->build());

    m_logger->info("Reading VIN");
    response = call(ServiceRequest_ReadDataByIdentifier::build()
                        ->id(DataIdentifier::VIN)
                        ->build());
    std::string VIN = ::Util::vec_to_str(
        static_cast<ServiceResponse_ReadDataByIdentifier*>(response.get())
            ->get_data()
            ->get_value());
    m_logger->info("VIN = " + VIN);
}

bool ReadWriteTask::is_completed() { return m_step == 6; }
