#include "configure_task/task.h"

#include "configure_task/config.h"
#include "configure_task/fields.h"
#include "service_all.h"
#include "security.h"

#include "configure_task/configuration_window.h"

ConfigurationTask::ConfigurationTask(std::shared_ptr<QLogger> logger, bool security, ConfigurationWindow* window)
    : QTask(logger), m_security(security) {

    connect(this, &ConfigurationTask::clear_errors_done, window, &ConfigurationWindow::clear_errors_done);
    connect(this, &ConfigurationTask::read_done, window, &ConfigurationWindow::read_done);
    connect(this, &ConfigurationTask::read_errors_done, window, &ConfigurationWindow::read_errors_done);
    connect(window, &ConfigurationWindow::factory_reset, this, &ConfigurationTask::factory_reset);
    connect(window, &ConfigurationWindow::read, this, &ConfigurationTask::read);
    connect(window, &ConfigurationWindow::write, this, &ConfigurationTask::write);
    connect(window, &ConfigurationWindow::read_errors, this, &ConfigurationTask::read_errors);
    connect(window, &ConfigurationWindow::clear_errors, this, &ConfigurationTask::clear_errors);
    connect(window, &ConfigurationWindow::closed, [this]() {
        emit windows_closed();
    });
}

ConfigurationTask::~ConfigurationTask() {

}

void ConfigurationTask::factory_reset() {
    diagnostic_session();
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
        return;
    }
    m_logger->info("Factory reset done");
}

void ConfigurationTask::task() {
    if(!diagnostic_session()) {
        return;
    }
    if(m_security) {
        if(!security_access(Crypto::SecuritySettings::get_mask03())) {
            return;
        }
    }

    QEventLoop loop;
    connect(this, &ConfigurationTask::windows_closed, &loop, &QEventLoop::quit);
    loop.exec();
}

void ConfigurationTask::clear_errors() {
    auto response = call(Can::ServiceRequest::ClearDiagnosticInformation::build()
                         ->group(0xFFFFFF)
                         ->build()
                         .value());
    IF_NEGATIVE(response) {
        m_logger->error("Failed to clear errors");
        emit clear_errors_done(false);
        return;
    }
    emit clear_errors_done(true);
}

void ConfigurationTask::read_errors(uint8_t mask) {
    auto response =
        call(Can::ServiceRequest::ReadDTCInformation::build()
             ->subfunction(Can::ServiceRequest::ReadDTCInformation::
                           Subfunction::reportDTCByStatusMask)
             ->mask(mask)
             ->build()
             .value());
    IF_NEGATIVE(response) {
        m_logger->error("Failed to read errors");
        return;
    }
    emit read_errors_done(mask, std::static_pointer_cast<Can::ServiceResponse::ReadDTCInformation>(response)->get_records());
}

bool ConfigurationTask::diagnostic_session() {
    auto response =
        call(Can::ServiceRequest::DiagnosticSessionControl::build()
             ->subfunction(Can::ServiceRequest::DiagnosticSessionControl::
                           Subfunction::extendDiagnosticSession)
             ->build()
             .value());

    IF_NEGATIVE(response) {
        m_logger->error("Failed ot enter extendDiagnosticSession");
        return false;
    }
    return true;
}

void ConfigurationTask::write(uint16_t id, std::vector<uint8_t> vec) {
    diagnostic_session();
    auto data = Can::Data::build()->raw_type(id)->value(vec)->build();
    auto request = Can::ServiceRequest::WriteDataByIdentifier::build()
        ->data(data.value())
        ->build();
    std::shared_ptr<Can::ServiceResponse::ServiceResponse> response =
        call(request.value());

    IF_NEGATIVE(response) {
        m_logger->error("Failed to write data");
    }
}

void ConfigurationTask::read(uint16_t id) {
    diagnostic_session();
    auto request =
        Can::ServiceRequest::ReadDataByIdentifier::build()->raw_id(id)->build();
    std::shared_ptr<Can::ServiceResponse::ServiceResponse> response =
        call(request.value());
    IF_NEGATIVE(response) {
        m_logger->error("Failed to read data");
        return;
    }
    emit read_done(
        id,
        std::static_pointer_cast<Can::ServiceResponse::ReadDataByIdentifier>(
            response)
            ->get_data()
            ->get_value());
}
