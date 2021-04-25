#include "task.h"

#include "config.h"
#include "fields.h"
#include "service_all.h"
#include "security.h"

#include "configuration_window.h"

ConfigurationTask::ConfigurationTask(std::shared_ptr<QLogger> logger, bool security)
    : QTask(logger), m_security(security) {
}

ConfigurationTask::~ConfigurationTask() {

}

bool ConfigurationTask::factory_reset() {
    auto response = call(
        Can::ServiceRequest::RoutineControl::build()
        ->subfunction(
            Can::ServiceRequest::RoutineControl::Subfunction::StartRoutine)
        ->routine(Can::Routine::build()
                  ->id(0x201f) // FIXME Factory reset
                  ->data({})
                  ->build()
                  .value())
        ->build()
        .value());
    IF_NEGATIVE(response) {
        m_logger->error("Failed to perform Factory reset");
        return false;
    }
    m_logger->info("Factory reset done");
    return true;
}

void ConfigurationTask::task() {
    if(m_security) {
        if(!security_access(Crypto::SecuritySettings::get_mask03())) {
            return;
        }
    }

    ConfigurationWindow window(nullptr, this);
    // window.setWindowModality(Qt::WindowModal);
    // QEventLoop loop;
    // connect(&window, &ConfigurationWindow::closed, &loop, &QEventLoop::quit);
    // window.show();
    window.exec();
    // loop.exec();
    // m_window->setFocus();
}

bool ConfigurationTask::clear_errors() {
    auto response = call(Can::ServiceRequest::ClearDiagnosticInformation::build()
                         ->group(0xFFFFFF)
                         ->build()
                         .value());
    IF_NEGATIVE(response) {
        m_logger->error("Failed to clear errors");
        return false;
    }
    return true;
}

optional<std::vector<std::shared_ptr<Can::DTC>>> ConfigurationTask::read_errors(uint8_t mask) {
    auto response =
        call(Can::ServiceRequest::ReadDTCInformation::build()
             ->subfunction(Can::ServiceRequest::ReadDTCInformation::
                           Subfunction::reportDTCByStatusMask)
             ->mask(mask)
             ->build()
             .value());
    IF_NEGATIVE(response) {
        m_logger->error("Failed to read errors");
        return {};
    }
    return std::static_pointer_cast<Can::ServiceResponse::ReadDTCInformation>(response)->get_records();
}

void ConfigurationTask::diagnostic_session() {
    auto response =
        call(Can::ServiceRequest::DiagnosticSessionControl::build()
             ->subfunction(Can::ServiceRequest::DiagnosticSessionControl::
                           Subfunction::extendDiagnosticSession)
             ->build()
             .value());

    IF_NEGATIVE(response) {
        m_logger->error("Failed ot enter extendDiagnosticSession");
        return;
    }
}

void ConfigurationTask::write(uint16_t id, std::vector<uint8_t> vec) {
    diagnostic_session();
    auto data = Can::Data::build()->raw_type(id)->value(vec)->build();
    auto request = Can::ServiceRequest::WriteDataByIdentifier::build()
        ->data(data.value())
        ->build();
    std::shared_ptr<Can::ServiceResponse::ServiceResponse> response =
        call(request.value());

    if (response->get_type() == Can::ServiceResponse::Type::Negative) {
        m_logger->error("Failed to write data");
    }
}

optional<std::vector<uint8_t>> ConfigurationTask::read(uint16_t id) {
    diagnostic_session();
    auto request =
        Can::ServiceRequest::ReadDataByIdentifier::build()->raw_id(id)->build();
    std::shared_ptr<Can::ServiceResponse::ServiceResponse> response =
        call(request.value());
    if (response->get_type() == Can::ServiceResponse::Type::Negative) {
        m_logger->error("Failed to read data");
        return {};
    }
    return std::static_pointer_cast<Can::ServiceResponse::ReadDataByIdentifier>(
        response)
        ->get_data()
        ->get_value();
}
