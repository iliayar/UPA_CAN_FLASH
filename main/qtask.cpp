#include "qtask.h"

#include <QDateTime>
#include <QProgressBar>
#include <QScrollBar>
#include <QSignalSpy>
#include <memory>

#include "frame_all.h"
#include "service_all.h"

void QTask::response(std::shared_ptr<Can::ServiceResponse> r, int wait) {
    if (wait == 1) {
        m_wait = std::max(wait, 1);
        return;
    } else if (wait > 1) {
        m_wait += wait;
    }
    m_response = r;
    emit response_imp(r);
}

std::shared_ptr<Can::ServiceResponse> QTask::call(
    std::shared_ptr<Can::ServiceRequest> req) {
    DEBUG(info, "QTask emit request");
    int retries = 1;
    emit request(req);
    while (1) {
        QSignalSpy spy(this, &QTask::response_imp);
        bool res = spy.wait(RESPONSE_TIMEOUT);
        if (!res) {
            if (m_wait) {
                m_wait--;
                continue;
            }
            retries++;
            if (retries > 3) {
                return std::make_shared<Can::ServiceResponse_Negative>(
                    req->get_type(), 0x00);
            }
            emit request(req);
            m_logger->warning("Service response timed out");
            continue;
        }
        if (m_response == nullptr) {
            m_logger->error("Invalid service response received");
            continue;
        }
        if (res) {
            if (m_response->get_type() == Can::ServiceResponseType::Negative) {
                if (static_cast<Can::ServiceResponse_Negative*>(
                        m_response.get())
                        ->get_service() != req->get_type()) {
                    DEBUG(info, "task response invalid error service code");
                    m_logger->warning("Invalid error service code");
                    continue;
                }
                if (static_cast<Can::ServiceResponse_Negative*>(
                        m_response.get())
                        ->get_code() == 0x78) {
                    DEBUG(info, "task response error service code = 0x78");
                    m_logger->warning("Waiting for positive resposnse");
                    m_wait += 4;
                    continue;
                }
            } else if (m_response->get_type() !=
                       Can::request_to_response_type(req->get_type())) {
                DEBUG(info, "task response code wrong");
                m_logger->warning("Invalid response code");
                continue;
            }
            return m_response;
        }
    }
}

using namespace Can;

void QTestTask::task() {
    std::shared_ptr<ServiceResponse> response;
    m_logger->info("Reading UPASystemType");
    response = call(ServiceRequest_ReadDataByIdentifier::build()
                        ->id(DataIdentifier::UPASystemType)
                        ->build());
    IF_NEGATIVE(response) {
        m_logger->error("Negative ReadDataByIdentifier response");
        return;
    }
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
    IF_NEGATIVE(response) {
        m_logger->error("Negative ReadDataByIdentifier response");
        return;
    }
    std::string VIN = ::Util::vec_to_str(
        static_cast<ServiceResponse_ReadDataByIdentifier*>(response.get())
            ->get_data()
            ->get_value());
    m_logger->info("VIN = " + VIN);
}
