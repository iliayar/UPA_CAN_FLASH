#include "service.h"

#include <memory>
#include <stdexcept>

#include "bytes.h"
#include "service_all.h"

std::shared_ptr<Can::ServiceResponse> Can::ServiceResponseFactory::get() {
    ServiceResponseType type =
        static_cast<ServiceResponseType>(m_reader.read_8(, 8));

    switch (type) {
#define SERVICE_BEGIN                       \
    case Can::ServiceResponseType::SERVICE: \
        return CONCAT(parse_, SERVICE)();
#include "services/services.h"
#undef SERVICE_BEGIN
        case Can::ServiceResponseType::Negative:
            return parse_Negative();
        default:
            return nullptr;
    }
#undef SUBFUNCTIONS
#undef DATATYPE
}
