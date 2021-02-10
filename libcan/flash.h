#pragma once

#include <vector>
#include <iostream>
#include <iomanip>
#include <string>
#include <chrono>

#include "communicator.h"
#include "task.h"

namespace Can {
    
// class FramesCSVLogger : public Logger {
// public:

//     FramesCSVLogger()
//         :m_start(std::chrono::high_resolution_clock::now()){
//     }
    
//     void transmitted_frame(Frame* frame) override {
//         std::cout << "Tester request: ";
//         print_frame(frame);
//     }

//     void recevied_frame(Frame* frame) override {
//         std::cout << "ECU response:   ";
//         print_frame(frame);
//     }

//     void received_service_response(ServiceResponse* r) override {
//         // std::cout << "Received response " << (int)r->get_type() << std::endl;
//     }

//     void transmitted_service_request(ServiceRequest* r) override {
//         // std::cout << "Transmited request " << (int)r->get_type() << std::endl;
//     }

//     // void error(std::string s) override {
//     //     std::cout << "ERROR: " << s << std::endl;
//     // }

// private:
//     void print_frame(Frame* frame) {
//         std::vector<uint8_t> payload = frame->dump();
//         std::cout << std::setfill(' ') << std::dec << std::setw(4) << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_start).count() << " | ";
//         for (uint8_t b : payload) {
//             std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)b;
//             std::cout << " ";
//         }
//         std::cout << std::endl;
//     }

//     std::chrono::high_resolution_clock::time_point m_start;
// };

class FramesStdLogger : public Logger {
public:

    FramesStdLogger()
        :m_start(std::chrono::high_resolution_clock::now()){
    }
    
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

    void received_service_response(ServiceResponse* r) override {
        // std::cout << "Received response " << (int)r->get_type() << std::endl;
    }

    void transmitted_service_request(ServiceRequest* r) override {
        // std::cout << "Transmited request " << (int)r->get_type() << std::endl;
    }

    void error(std::string s) override {
        std::cout << "ERROR: " << s << std::endl;
    }

    void info(std::string s) override {
    }
    void warning(std::string s) override {
    }

private:
    void print_frame(std::shared_ptr<Frame> frame) {
        std::vector<uint8_t> payload = frame->dump();
        std::cout << std::setfill(' ') << std::dec << std::setw(4) << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - m_start).count() << " | ";
        for (uint8_t b : payload) {
            std::cout << std::setw(2) << std::setfill('0') << std::hex << (int)b;
            std::cout << " ";
        }
        std::cout << std::endl;
    }

    std::chrono::high_resolution_clock::time_point m_start;
};

class FlashTask : public AsyncTask {
public:
    void task();
};

}  // namespace Can
