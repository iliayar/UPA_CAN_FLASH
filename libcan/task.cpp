#include "task.h"

#include <iostream>
#include <string>
#include <vector>

#include "service_all.h"
#include "util.h"

using namespace Can;

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
    response = call(
	new ServiceRequest_ReadDataByIdentifier(DataIdentifier::UPASystemType));
    uint8_t type = static_cast<ServiceResponse_ReadDataByIdentifier*>(response)
		       ->get_data()
		       ->get_value()[0];
    std::cout << "UPASystemType = " << (int)type << std::endl;

    call(new ServiceRequest_WriteDataByIdentifier(new Data(
	DataIdentifier::VIN, ::Util::str_to_vec("HELLO ANYBODY ..."))));

    response =
        call(new ServiceRequest_ReadDataByIdentifier(DataIdentifier::VIN));
    std::string VIN = ::Util::vec_to_str(
        static_cast<ServiceResponse_ReadDataByIdentifier*>(response)
            ->get_data()
            ->get_value());
    std::cout << "VIN = " << VIN << std::endl;
}

bool ReadWriteTask::is_completed() { return m_step == 6; }
