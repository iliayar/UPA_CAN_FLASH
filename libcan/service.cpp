#include "service.h"

#include <memory>
#include <stdexcept>

#include "bytes.h"
#include "service_all.h"


Can::ServiceResponse::Factory::Factory(std::vector<uint8_t> payload)
    : m_reader(payload)
{}


optional<std::shared_ptr<Can::ServiceResponse::ServiceResponse>> Can::ServiceResponse::Factory::get() {
    m_type.read(m_reader);
    if(!m_type.valid()) {
        return {};
    }
    switch (m_type.get().value()) {
        case Can::ServiceResponse::Type::Negative:
            return Can::ServiceResponse::Negative::build(m_reader)->build();
        default:
            return {};
    }
}
