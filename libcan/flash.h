#pragma once

#include "communicator.h"

#include <vector>

namespace Can {

	class FramesStdLogger : public Logger {
	public:
		void transmitted_frame(Frame* frame) override {
			print_frame(frame);
		}
		
		void recevied_frame(Frame* frame) override {
			print_frame(frame);
		}

		void received_service_response(ServiceResponse *) override {
		}

		void transmitted_serviec_request(ServiceRequest *) override {

		}
	private:
		void print_frame(Frame* frame) {
			std::vector<uint8_t> payload = frame->dump();
			for(uint8_t b : payload) {
				std::cout << std::hex << b;
				std::cout << " ";
			}
			std::cout << std::endl;
		}
	};

}
