#include "qtask.h"

#include <QDateTime>
#include <QProgressBar>
#include <QScrollBar>
#include <QSignalSpy>
#include <memory>

#include "frame_all.h"
#include "service_all.h"
#include "security.h"


namespace {
std::shared_ptr<Can::ServiceResponse::ServiceResponse> negative(
    std::shared_ptr<Can::ServiceRequest::ServiceRequest> req) {
    return Can::ServiceResponse::Negative::build()
        ->service(req->get_type())
        ->code(0x00)
        ->build()
        .value();
}
}  // namespace

QTask::QTask(std::shared_ptr<QLogger> logger) : m_logger(logger), m_wait(0) {}

void QTask::run() {
    try {
        task();
    } catch (std::experimental::bad_optional_access const& e) {
        m_logger->error(std::string("Task failed: ") + e.what());
    } catch (interrupted_exception const& e) {
        m_logger->error("Task was interrupted");
    }
    DEBUG(info, "Exiting task");
}

void QTask::response(std::shared_ptr<Can::ServiceResponse::ServiceResponse> r,
                     int wait) {
    if (wait == 1) {
        m_wait = std::max(wait, 1);
        return;
    } else if (wait > 1) {
        m_wait += wait;
    }
    m_response = r;
    if(!m_is_interrupted) {
        emit response_imp(r);
    }
}

std::shared_ptr<Can::ServiceResponse::ServiceResponse> QTask::call(
    std::shared_ptr<Can::ServiceRequest::ServiceRequest> req) {
    DEBUG(info, "QTask emit request");
    int retries = 1;
    if(!m_is_interrupted) {
        emit request(req);
    }
    while (1) {
        QSignalSpy spy(this, &QTask::response_imp);
        bool res = spy.wait(RESPONSE_TIMEOUT);
        if (m_is_interrupted) {
            throw interrupted_exception();
        }
        if (!res) {
            if (m_wait) {
                m_wait--;
                continue;
            }
            retries++;
            if (retries > 3) {
                interrupt();
                throw interrupted_exception();
                // return negative(req);
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
            if (m_response->get_type() == Can::ServiceResponse::Type::Negative) {
                if (std::static_pointer_cast<Can::ServiceResponse::Negative>(
                        m_response)
                    ->get_service() != req->get_type()) {
                    DEBUG(info, "task response invalid error service code");
                    m_logger->warning("Invalid error service code");
                    continue;
                }
                if (std::static_pointer_cast<Can::ServiceResponse::Negative>(
                        m_response)
                    ->get_code() == 0x78) {
                    DEBUG(info, "task response error service code = 0x78");
                    m_logger->warning("Waiting for positive response");
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

void QTask::interrupt() {
    m_is_interrupted = true;
}

using namespace Can;

bool QTask::security_access(uint32_t mask) {
    uint8_t rnd = Crypto::get_RND();

    m_logger->info("Seed parameter " + Util::int_to_hex(rnd));
    auto response =
        call(ServiceRequest::SecurityAccess::build()
                 ->subfunction(
                     ServiceRequest::SecurityAccess::Subfunction::requestSeed)
                 ->seed_par(rnd)
                 ->build()
                 .value());

    IF_NEGATIVE(response) {
        LOG(error, "Failed ot request seed");
        return false;
    }

    uint32_t seed =
        std::static_pointer_cast<ServiceResponse::SecurityAccess>(response)
            ->get_seed();

    LOG(info, "Received seed " + Util::int_to_hex(seed));

    uint32_t key = Crypto::seed_to_key(seed, rnd, mask);

    LOG(info, "Calculated key " + Util::int_to_hex(key));

    response = call(
        ServiceRequest::SecurityAccess::build()
            ->subfunction(ServiceRequest::SecurityAccess::Subfunction::sendKey)
            ->key(key)
            ->build()
            .value());

    IF_NEGATIVE(response) {
        LOG(error, "Security access failed key verification");
        return false;
    }

    LOG(info, "Successfully passed security access");
    return true;
}
