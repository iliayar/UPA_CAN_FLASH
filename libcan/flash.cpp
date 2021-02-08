#include "flash.h"
#include "service_all.h"
#include "security.h"

#define IF_NEGATIVE(res) if(res->get_type() == Can::ServiceResponseType::Negative)

void Can::FlashTask::task() {
    ServiceResponse* response;

    response = call(new ServiceRequest_DiagnosticSessionControl(
        DiagnosticSessionControl_SubfunctionType::extendDiagnosticSession));

    IF_NEGATIVE(response) {
        std::cout << "Failed ot enter extendDiagnosticSession" << std::endl;
    }

    response = call(new ServiceRequest_ControlDTCSettings(
        ControlDTCSettings_SubfunctionType::off));

    IF_NEGATIVE(response) {
        std::cout << "Failed ControlDTCSettings" << std::endl;
    }

    response = call(new ServiceRequest_CommunicationControl(
        CommunicationControl_SubfunctionType::disableRxAndTx, 0x03));

    IF_NEGATIVE(response) {
        std::cout << "Failed CommunicationControl" << std::endl;
    }

    response = call(new ServiceRequest_DiagnosticSessionControl(
        DiagnosticSessionControl_SubfunctionType::programmingSession));

    IF_NEGATIVE(response) {
        std::cout << "Failed ot enter programmingSession" << std::endl;
    }

    uint8_t rnd = Crypto::get_RND();
    response = call(new ServiceRequest_SecurityAccess(
        SecurityAccess_SubfunctionType::requestSeed, rnd, 0));

    std::cout << "Seed parameter " << (int)rnd << std::endl;

    IF_NEGATIVE(response) {
        std::cout << "Failed ot request seed" << std::endl;
    }

    if(response->get_type() != ServiceResponseType::SecurityAccess) {
        std::cout << "Invalid service response" << std::endl;
        return;
    }
    if(static_cast<ServiceResponse_SecurityAccess*>(response)->get_subfunction() != SecurityAccess_SubfunctionType::requestSeed) {
        std::cout << "Invalid subfunciton" << std::endl;
        return;
    }

    uint32_t seed =
        static_cast<ServiceResponse_SecurityAccess*>(response)->get_seed();

    std::cout << "Received seed ";
    std::cout << std::hex << seed << std::endl;

    uint32_t key = Crypto::seed_to_key_02(seed, rnd);

    std::cout << "Calculated key ";
    std::cout << std::hex << key << std::endl;

    response = call(new ServiceRequest_SecurityAccess(SecurityAccess_SubfunctionType::sendKey, 0, key));

    IF_NEGATIVE(response) {
        std::cout << "Security access failed key verification" << std::endl;
    }

    std::cout << "Successfully pased security access" << std::endl;
}
