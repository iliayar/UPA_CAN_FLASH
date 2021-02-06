#pragma once

#include "service.h"

namespace Can {

    class Task {
    public:
        virtual ServiceRequest* fetch_request() = 0;
        virtual void push_response(ServiceResponse*) = 0;
    };

}
