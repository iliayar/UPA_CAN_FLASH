#include "task.h"

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

#include "service_all.h"
#include "util.h"

using namespace Can;

Can::ServiceResponse* Can::AsyncTask::call_imp(Can::ServiceRequest* request) {
    while (true) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_request == nullptr) {
                m_request = request;
                m_wait_response = true;
                break;
            }
        }
    }
    while (true) {
        {
            std::unique_lock<std::mutex> lock(m_mutex);
            if (m_response != nullptr) {
                Can::ServiceResponse* response = m_response;
                if (response->get_type() ==
                    Can::ServiceResponseType::Negative) {
                    if (static_cast<Can::ServiceResponse_Negative*>(response)
                            ->get_service() != request->get_type()) {
                        m_response = nullptr;
                        m_logger->warning("Invalid error service code");
                        continue;
                    }
                    if (static_cast<Can::ServiceResponse_Negative*>(response)
                            ->get_code() == 0x78) {
                        m_response = nullptr;
                        m_logger->warning("Waiting for positive resposnse");
                        continue;
                    }
                } else if (response->get_type() !=
                           Can::request_to_response_type(request->get_type())) {
                    m_response = nullptr;
                    m_logger->warning("Invalid response code");
                    continue;
                }
                m_response = nullptr;
                m_wait_response = false;
                return response;
            }
        }
    }
}

ServiceRequest* ReadWriteTask::fetch_request() {
    m_step++;
    switch (m_step - 1) {
	case 0:
	    return new ServiceRequest_ReadDataByIdentifier(
		DataIdentifier::UPASystemType);
	case 2:
	    return new ServiceRequest_WriteDataByIdentifier(new Data(
		DataIdentifier::VIN, ::Util::str_to_vec("HELLO ANYBODY ...")));
	case 4:
	    return new ServiceRequest_ReadDataByIdentifier(DataIdentifier::VIN);
	default:
	    m_step--;
	    return nullptr;
    }
}

void ReadWriteTask::push_response(ServiceResponse* response) {
    m_step++;
    switch (m_step - 1) {
	case 1: {
	    uint8_t type =
		static_cast<ServiceResponse_ReadDataByIdentifier*>(response)
		    ->get_data()
		    ->get_value()[0];
	    delete response;
	    std::cout << "UPASystemType = " << (int)type << std::endl;
	    break;
	}
	case 3: {
	    delete response;
	    break;
	}
	case 5: {
	    std::string VIN = ::Util::vec_to_str(
		static_cast<ServiceResponse_ReadDataByIdentifier*>(response)
		    ->get_data()
		    ->get_value());
	    delete response;
	    std::cout << "VIN = " << VIN << std::endl;
	    break;
	}
	default:
	    m_step--;
	    return;
    }
}

void ReadWriteThreadedTask::task() {
    ServiceResponse* response;
    m_logger->info("Reading UPASystemType");
    response = call(
	new ServiceRequest_ReadDataByIdentifier(DataIdentifier::UPASystemType));
    uint8_t type = static_cast<ServiceResponse_ReadDataByIdentifier*>(response)
		       ->get_data()
		       ->get_value()[0];
    std::stringstream ss;
    ss << "UPASystemType = " << std::hex << (int)type;
    m_logger->info(ss.str());

    m_logger->info("Writing VIN");
    call(new ServiceRequest_WriteDataByIdentifier(new Data(
	DataIdentifier::VIN, ::Util::str_to_vec("HELLO ANYBODY ..."))));

    m_logger->info("Reading VIN");
    response =
        call(new ServiceRequest_ReadDataByIdentifier(DataIdentifier::VIN));
    std::string VIN = ::Util::vec_to_str(
        static_cast<ServiceResponse_ReadDataByIdentifier*>(response)
            ->get_data()
            ->get_value());
    m_logger->info("VIN = " + VIN);
}

bool ReadWriteTask::is_completed() { return m_step == 6; }
