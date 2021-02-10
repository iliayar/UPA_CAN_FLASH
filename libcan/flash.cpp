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

    // response = call(new ServiceRequest_CommunicationControl(
    //     CommunicationControl_SubfunctionType::disableRxAndTx, 0x03));
    response = call(
        ServiceRequest_CommunicationControl::build()
            ->subfunction(CommunicationControl_SubfunctionType::disableRxAndTx)
            ->communication_type(CommunicationType::build()
                                     ->chanels(CommunicationTypeChanels::build()
                                                   ->network_communication(1)
                                                   ->normal_communication(1)
                                                   ->build())
                                     ->build())
            ->build());

    IF_NEGATIVE(response) {
        std::cout << "Failed CommunicationControl" << std::endl;
    }

    response = call(
        ServiceRequest_DiagnosticSessionControl::build()
            ->subfunction(
                DiagnosticSessionControl_SubfunctionType::programmingSession)
            ->build());

    IF_NEGATIVE(response) {
        std::cout << "Failed ot enter programmingSession" << std::endl;
    }

    uint8_t rnd = Crypto::get_RND();
    response =
        call(ServiceRequest_SecurityAccess::build()
                 ->subfunction(SecurityAccess_SubfunctionType::requestSeed)
                 ->seed_par(rnd)
                 ->build());

    std::cout << "Seed parameter " << (int)rnd << std::endl;

    IF_NEGATIVE(response) {
        std::cout << "Failed ot request seed" << std::endl;
    }

    uint32_t seed =
        static_cast<ServiceResponse_SecurityAccess*>(response)->get_seed();

    std::cout << "Received seed ";
    std::cout << std::hex << seed << std::endl;

    uint32_t key = Crypto::seed_to_key_02(seed, rnd);

    std::cout << "Calculated key ";
    std::cout << std::hex << key << std::endl;

    response = call(ServiceRequest_SecurityAccess::build()
                        ->subfunction(SecurityAccess_SubfunctionType::sendKey)
                        ->key(key)
                        ->build());

    IF_NEGATIVE(response) {
        std::cout << "Security access failed key verification" << std::endl;
    }

    std::cout << "Successfully pased security access" << std::endl;

}
