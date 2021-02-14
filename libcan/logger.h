#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <fstream>
#include <memory>
#include <string>
#include <vector>
#include <mutex>

#include "frame.h"
#include "service.h"

namespace Can {

    enum class LogLevel {
        FrameReceived,
        FrameTransmitted,
        SericesReceived,
        ServiceTransmitted,
        Error,
        Info,
        Warning
    };
    
class Logger {
public:
    virtual void received_frame(std::shared_ptr<Frame>) = 0;
    virtual void transmitted_frame(std::shared_ptr<Frame>) = 0;
    virtual void received_service_response(std::shared_ptr<ServiceResponse>) = 0;
    virtual void transmitted_service_request(std::shared_ptr<ServiceRequest>) = 0;
    virtual void error(std::string) = 0;
    virtual void info(std::string) = 0;
    virtual void warning(std::string) = 0;
    virtual void important(std::string) = 0;

    template<typename T>
    void log(LogLevel level, T data) {
        switch(level) {
        case LogLevel::Error:
            error(data);
            break;
        case LogLevel::Info:
            info(data);
            break;
        case LogLevel::Warning:
            warning(data);
            break;
        case LogLevel::FrameReceived:
            received_frame(data);
            break;
        case LogLevel::FrameTransmitted:
            transmitted_frame(data);
            break;
        case LogLevel::SericesReceived:
            received_service_response(data);
            break;
        case LogLevel::ServiceTransmitted:
            transmitted_service_request(data);
            break;
        }
    }
};

class NoLogger : public Logger {
public:
    void received_frame(std::shared_ptr<Frame> _) {}
    void transmitted_frame(std::shared_ptr<Frame> _) {}
    void received_service_response(std::shared_ptr<ServiceResponse> _) {}
    void transmitted_service_request(std::shared_ptr<ServiceRequest> _) {}
    void error(std::string _) {}
    void info(std::string _) {}
    void warning(std::string _) {}
    void important(std::string _) {}
};

class FramesStdLogger : public Logger {
public:
    FramesStdLogger() : m_start(std::chrono::high_resolution_clock::now()) {}

    void transmitted_frame(std::shared_ptr<Frame> frame) override {
        std::cout << "Tester request: ";
        print_frame(frame);
        m_start = std::chrono::high_resolution_clock::now();
    }

    void received_frame(std::shared_ptr<Frame> frame) override {
        std::cout << "ECU response:   ";
        print_frame(frame);
        m_start = std::chrono::high_resolution_clock::now();
    }

    void received_service_response(std::shared_ptr<ServiceResponse> r) override {
        // std::cout << "Received response " << (int)r->get_type() << std::endl;
    }

    void transmitted_service_request(std::shared_ptr<ServiceRequest> r) override {
        // std::cout << "Transmited request " << (int)r->get_type() <<
        // std::endl;
    }

    void error(std::string s) override {
        std::cout << "ERROR: " << s << std::endl;
    }

    void info(std::string s) override {
        std::cout << "INFO: " << s << std::endl;
    }
    void warning(std::string s) override {
        std::cout << "WARNING: " << s << std::endl;
    }
    void important(std::string s) override {
        info(s);
    }

private:
    void print_frame(std::shared_ptr<Frame> frame) {
        std::vector<uint8_t> payload = frame->dump();
        std::cout << std::setfill(' ') << std::dec << std::setw(4)
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::high_resolution_clock::now() - m_start)
                         .count()
                  << " | ";
        for (uint8_t b : payload) {
            std::cout << std::setw(2) << std::setfill('0') << std::hex
                      << (int)b;
            std::cout << " ";
        }
        std::cout << std::endl;
    }

    std::chrono::high_resolution_clock::time_point m_start;
};

class FileLogger  : public Logger {
public:
    FileLogger(std::string file) : m_start(std::chrono::high_resolution_clock::now()), m_fout(file) {
        if(!m_fout) throw std::runtime_error("Cannot open log file");
    }

    void transmitted_frame(std::shared_ptr<Frame> frame) override {
        std::unique_lock<std::mutex> lock(m_mutex); 
        m_fout << "Tester request: ";
        print_frame(frame);
        m_start = std::chrono::high_resolution_clock::now();
    }

    void received_frame(std::shared_ptr<Frame> frame) override {
        std::unique_lock<std::mutex> lock(m_mutex); 
        m_fout << "ECU response:   ";
        print_frame(frame);
        m_start = std::chrono::high_resolution_clock::now();
    }

    void received_service_response(std::shared_ptr<ServiceResponse> r) override {
        // m_fout << "Received response " << (int)r->get_type() << std::endl;
    }

    void transmitted_service_request(std::shared_ptr<ServiceRequest> r) override {
        // m_fout << "Transmited request " << (int)r->get_type() <<
        // std::endl;
    }

    void error(std::string s) override {
        std::unique_lock<std::mutex> lock(m_mutex); 
        m_fout << "ERROR: " << s << std::endl;
        m_fout.flush();
    }

    void info(std::string s) override {
        std::unique_lock<std::mutex> lock(m_mutex); 
        m_fout << "INFO: " << s << std::endl;
        m_fout.flush();
    }
    void warning(std::string s) override {
        std::unique_lock<std::mutex> lock(m_mutex); 
        m_fout << "WARNING: " << s << std::endl;
        m_fout.flush();
    }
    void important(std::string s) override {
        info(s);
    }
     
    ~FileLogger() {
        m_fout.close();

    }

private:
    void print_frame(std::shared_ptr<Frame> frame) {
        std::vector<uint8_t> payload = frame->dump();
        m_fout << std::setfill(' ') << std::dec << std::setw(4)
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::high_resolution_clock::now() - m_start)
                         .count()
                  << " | ";
        for (uint8_t b : payload) {
            m_fout << std::setw(2) << std::setfill('0') << std::hex
                      << (int)b;
            m_fout << " ";
        }
        m_fout << std::endl;
    }

    std::chrono::high_resolution_clock::time_point m_start;
    std::ofstream m_fout;
    std::mutex m_mutex;
};

class GlobalLogger {
    public:
    static Logger* instance();
    private:
        static Logger* m_logger;
};

#define DEBUG(level, str) Can::GlobalLogger::instance()->level(std::string{} + __FILE__ + ":" + std::to_string(__LINE__) + " " + __PRETTY_FUNCTION__ + " " + str);

}  // namespace Can
