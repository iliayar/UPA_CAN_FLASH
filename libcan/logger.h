#pragma once

#include "frame.h"
#include "service.h"

#include <string>
#include <memory>

namespace Can {

class Logger {
public:
    virtual void received_frame(std::shared_ptr<Frame>) = 0;
    virtual void transmitted_frame(std::shared_ptr<Frame>) = 0;
    virtual void received_service_response(ServiceResponse*) = 0;
    virtual void transmitted_service_request(ServiceRequest*) = 0;
    virtual void error(std::string) = 0;
    virtual void info(std::string) = 0;
    virtual void warning(std::string) = 0;
};

class NoLogger : public Logger {
public:
    void received_frame(std::shared_ptr<Frame> _) {}
    void transmitted_frame(std::shared_ptr<Frame> _) {}
    void received_service_response(ServiceResponse* _) {}
    void transmitted_service_request(ServiceRequest* _) {}
    void error(std::string _) {}
    void info(std::string _) {}
    void warning(std::string _) {}
};

}
