#pragma once

#include "service.h"

namespace Can {

    class Task {
    public:
        virtual Service* fetch_request() = 0;
        virtual void push_response(Service*) = 0;
    };

}
