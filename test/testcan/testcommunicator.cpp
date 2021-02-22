#include "gtest/gtest.h"
#include "service_all.h"
#include "frame_all.h"
#include "communicator.h"
#include "task.h"
#include "util.h"

#include <vector>
#include <chrono>
#include <thread>
#include <string>
#include <stdexcept>
#include <memory>

TEST(testCommunicator, testFramesToService)
{
    {
        std::vector<std::shared_ptr<Can::Frame>> frames;
        frames.push_back(std::make_shared<Can::Frame_SingleFrame>(3, std::vector<uint8_t>({0x6e, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00})));
        std::shared_ptr<Can::ServiceResponse> response = Can::frames_to_service(frames);
        EXPECT_EQ(response->get_type(), Can::ServiceResponseType::WriteDataByIdentifier);
        Can::DataIdentifier id = static_cast<Can::ServiceResponse_WriteDataByIdentifier*>(response.get())->get_id();
        EXPECT_EQ(id, Can::DataIdentifier::VIN);
    }

    {
        std::vector<std::shared_ptr<Can::Frame>> frames;
        frames.push_back(std::make_shared<Can::Frame_FirstFrame>(20, std::vector<uint8_t>({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})));
        frames.push_back(std::make_shared<Can::Frame_ConsecutiveFrame>(1, std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})));
        frames.push_back(std::make_shared<Can::Frame_ConsecutiveFrame>(2, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        std::shared_ptr<Can::ServiceResponse> response = Can::frames_to_service(frames);
        EXPECT_EQ(response->get_type(), Can::ServiceResponseType::ReadDataByIdentifier);
        Can::Data* data = static_cast<Can::ServiceResponse_ReadDataByIdentifier*>(response.get())->get_data();
        EXPECT_EQ(data->get_type(), Can::DataIdentifier::VIN);
        EXPECT_EQ(data->get_value(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }
}

TEST(testCommunicator, testServiceToFrames)
{
    {
        std::shared_ptr<Can::ServiceRequest> request = std::make_shared<Can::ServiceRequest_WriteDataByIdentifier>(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        std::vector<std::shared_ptr<Can::Frame>> frames = Can::service_to_frames(request);
        EXPECT_EQ(frames.size(), 3);
        EXPECT_EQ(frames[0]->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frames[0].get())->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frames[0].get())->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(frames[1]->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frames[1].get())->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frames[1].get())->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frames[2].get())->get_seq_num(), 2);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frames[2].get())->get_data(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    {
        std::shared_ptr<Can::ServiceRequest> request = std::make_shared<Can::ServiceRequest_ReadDataByIdentifier>(Can::DataIdentifier::VIN);
        std::vector<std::shared_ptr<Can::Frame>> frames = Can::service_to_frames(request);
        EXPECT_EQ(frames.size(), 1);
        EXPECT_EQ(frames[0]->get_type(), Can::FrameType::SingleFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frames[0].get())->get_len(), 3);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frames[0].get())->get_data(), std::vector<uint8_t>({0x22, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00}));
    }
}

TEST(testCommunicator, testReceiver)
{
    {
        std::vector<std::shared_ptr<Can::Frame>> frames;
        frames.push_back(std::make_shared<Can::Frame_SingleFrame>(3, std::vector<uint8_t>({0x6e, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00})));
        Can::Receiver receiver(frames[0]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Done);
        std::shared_ptr<Can::ServiceResponse> response = receiver.get_response();
        EXPECT_EQ(response->get_type(), Can::ServiceResponseType::WriteDataByIdentifier);
        Can::DataIdentifier id = static_cast<Can::ServiceResponse_WriteDataByIdentifier*>(response.get())->get_id();
        EXPECT_EQ(id, Can::DataIdentifier::VIN);
    }

    {
        std::vector<std::shared_ptr<Can::Frame>> frames;
        frames.push_back(std::make_shared<Can::Frame_FirstFrame>(20, std::vector<uint8_t>({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})));
        frames.push_back(std::make_shared<Can::Frame_ConsecutiveFrame>(1, std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})));
        frames.push_back(std::make_shared<Can::Frame_ConsecutiveFrame>(2, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Receiver receiver(frames[0]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Work);
        std::shared_ptr<Can::Frame> flow_control = receiver.fetch_frame();
        EXPECT_EQ(flow_control->get_type(), Can::FrameType::FlowControl);
        receiver.push_frame(frames[1]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Work);
        receiver.push_frame(frames[2]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Done);
        std::shared_ptr<Can::ServiceResponse> response = receiver.get_response();
        EXPECT_EQ(response->get_type(), Can::ServiceResponseType::ReadDataByIdentifier);
        Can::Data* data = static_cast<Can::ServiceResponse_ReadDataByIdentifier*>(response.get())->get_data();
        EXPECT_EQ(data->get_type(), Can::DataIdentifier::VIN);
        EXPECT_EQ(data->get_value(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    { // Test timeout
        std::vector<std::shared_ptr<Can::Frame>> frames;
        frames.push_back(std::make_shared<Can::Frame_FirstFrame>(20, std::vector<uint8_t>({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})));
        frames.push_back(std::make_shared<Can::Frame_ConsecutiveFrame>(1, std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})));
        frames.push_back(std::make_shared<Can::Frame_ConsecutiveFrame>(2, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Receiver receiver(frames[0]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Work);
        std::shared_ptr<Can::Frame> flow_control = receiver.fetch_frame();
        EXPECT_EQ(flow_control->get_type(), Can::FrameType::FlowControl);
        receiver.TIMEOUT = std::chrono::milliseconds(50);
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Error);
    }
}


TEST(testCommunicator, testTransmitter)
{
    {
        std::shared_ptr<Can::ServiceRequest> request = std::make_shared<Can::ServiceRequest_ReadDataByIdentifier>(Can::DataIdentifier::VIN);
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::shared_ptr<Can::Frame> frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::SingleFrame);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(frame->get_type(), Can::FrameType::SingleFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_len(), 3);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x22, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00}));
    }

    {
        std::shared_ptr<Can::ServiceRequest> request = std::make_shared<Can::ServiceRequest_WriteDataByIdentifier>(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::shared_ptr<Can::Frame> frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(std::make_shared<Can::Frame_FlowControl>(Can::FlowStatus::ContinueToSend, 0, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_seq_num(), 2);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    { // Test BlockSize = 1
        std::shared_ptr<Can::ServiceRequest> request = std::make_shared<Can::ServiceRequest_WriteDataByIdentifier>(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Transmitter transmitter(request);
        transmitter.TIMEOUT = std::chrono::milliseconds(50);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::shared_ptr<Can::Frame> frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        transmitter.push_frame(std::make_shared<Can::Frame_FlowControl>(Can::FlowStatus::ContinueToSend, 1, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        transmitter.push_frame(std::make_shared<Can::Frame_FlowControl>(Can::FlowStatus::ContinueToSend, 1, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_seq_num(), 2);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    { // Test Timeout
        std::shared_ptr<Can::ServiceRequest> request = std::make_shared<Can::ServiceRequest_WriteDataByIdentifier>(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Transmitter transmitter(request);
        transmitter.TIMEOUT = std::chrono::milliseconds(50);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::shared_ptr<Can::Frame> frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(std::make_shared<Can::Frame_FlowControl>(Can::FlowStatus::ContinueToSend, 1, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Error);
    }

    { // Test FlowControl Repeat type
        std::shared_ptr<Can::ServiceRequest> request = std::make_shared<Can::ServiceRequest_WriteDataByIdentifier>(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::shared_ptr<Can::Frame> frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(std::make_shared<Can::Frame_FlowControl>(Can::FlowStatus::WaitForAnotherFlowControlMessageBeforeContinuing, 0, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(frame, nullptr);
        transmitter.push_frame(std::make_shared<Can::Frame_FlowControl>(Can::FlowStatus::ContinueToSend, 0, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_seq_num(), 2);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    { // Test Minimum separation time
        std::shared_ptr<Can::ServiceRequest> request = std::make_shared<Can::ServiceRequest_WriteDataByIdentifier>(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::shared_ptr<Can::Frame> frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(std::make_shared<Can::Frame_FlowControl>(Can::FlowStatus::ContinueToSend, 1, 30));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(frame, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        frame = transmitter.fetch_frame();
        EXPECT_NE(frame, nullptr);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        transmitter.push_frame(std::make_shared<Can::Frame_FlowControl>(Can::FlowStatus::ContinueToSend, 1, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_seq_num(), 2);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    {
        std::shared_ptr<Can::ServiceRequest> request = std::make_shared<Can::ServiceRequest_WriteDataByIdentifier>(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::shared_ptr<Can::Frame> frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(std::make_shared<Can::Frame_FlowControl>(Can::FlowStatus::ContinueToSend, 0, 30));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(frame, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        frame = transmitter.fetch_frame();
        EXPECT_NE(frame, nullptr);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(frame, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        frame = transmitter.fetch_frame();
        EXPECT_NE(frame, nullptr);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_seq_num(), 2);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }
}


namespace Can {

    class TestTask_Read : public Task {
    public:
        std::shared_ptr<ServiceRequest> fetch_request() {
            m_step++;
            switch(m_step - 1) {
            case 0:
                return std::make_shared<ServiceRequest_ReadDataByIdentifier>(DataIdentifier::VIN);
            default:
                m_step--;
                return nullptr;
            }
        }

        void push_response(std::shared_ptr<ServiceResponse> response) {
            m_step++;
            switch(m_step - 1) {
            case 1:
                m_result = Util::vec_to_str(static_cast<ServiceResponse_ReadDataByIdentifier*>(response.get())->get_data()->get_value());
                break;
            default:
                m_step--;
                return;
            }
        }

        std::string get_result() {
            return m_result;
        }

        bool is_completed() {
            return m_step == 2;
        }

    private:
        int m_step = 0;
        std::string m_result;
    };

    class TestTask_DoubleRead : public Task {
    public:
        std::shared_ptr<ServiceRequest> fetch_request() {
            m_step++;
            switch(m_step - 1) {
            case 0:
                return std::make_shared<ServiceRequest_ReadDataByIdentifier>(DataIdentifier::VIN);
            case 2:
                return std::make_shared<ServiceRequest_ReadDataByIdentifier>(DataIdentifier::UPASystemType);
            default:
                m_step--;
                return nullptr;
            }
        }

        void push_response(std::shared_ptr<ServiceResponse> response) {
            m_step++;
            switch(m_step - 1) {
            case 1:
                m_result_VIN = Util::vec_to_str(static_cast<ServiceResponse_ReadDataByIdentifier*>(response.get())->get_data()->get_value());
                break;
            case 3:
                m_result_UPASystemType = static_cast<ServiceResponse_ReadDataByIdentifier*>(response.get())->get_data()->get_value()[0];
                break;
            default:
                m_step--;
                return;
            }
        }

        std::pair<std::string, uint8_t> get_result() {
            return std::make_pair(m_result_VIN, m_result_UPASystemType);
        }

        bool is_completed() {
            return m_step == 4;
        }

    private:
        int m_step = 0;
        std::string m_result_VIN;
        uint8_t m_result_UPASystemType;
    };

}

TEST(testCommunicator, testTaskRead)
{
    Can::TestTask_Read* task = new Can::TestTask_Read();
    Can::Communicator communicator{};
    communicator.set_task(task);
    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Transmit);

    std::shared_ptr<Can::Frame> frame = communicator.fetch_frame();
    EXPECT_EQ(frame->get_type(), Can::FrameType::SingleFrame);
    EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_len(), 3);
    EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x22, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00}));

    std::vector<std::shared_ptr<Can::Frame>> frames;
    frames.push_back(std::make_shared<Can::Frame_FirstFrame>(20, std::vector<uint8_t>({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})));
    frames.push_back(std::make_shared<Can::Frame_ConsecutiveFrame>(1, std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})));
    frames.push_back(std::make_shared<Can::Frame_ConsecutiveFrame>(2, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));

    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Idle);
    for(std::shared_ptr<Can::Frame> f : frames) {
        communicator.push_frame(f);
    }
    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Idle);
    EXPECT_EQ(task->is_completed(), true);
    EXPECT_EQ(task->get_result(), "A A A A A A A A A");
}

TEST(testCommunicator, testTaskWriteRead)
{
    Can::TestTask_DoubleRead* task = new Can::TestTask_DoubleRead();
    Can::Communicator communicator{};
    communicator.set_task(task);
    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Transmit);

    std::shared_ptr<Can::Frame> frame = communicator.fetch_frame();
    EXPECT_EQ(frame->get_type(), Can::FrameType::SingleFrame);
    EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_len(), 3);
    EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x22, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00}));

    std::vector<std::shared_ptr<Can::Frame>> frames;
    frames.push_back(std::make_shared<Can::Frame_FirstFrame>(20, std::vector<uint8_t>({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})));
    frames.push_back(std::make_shared<Can::Frame_ConsecutiveFrame>(1, std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})));
    frames.push_back(std::make_shared<Can::Frame_ConsecutiveFrame>(2, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));

    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Idle);
    ASSERT_THROW(communicator.fetch_frame(), Can::NothingToFetch);
    for(std::shared_ptr<Can::Frame> f : frames) {
        communicator.push_frame(f);
    }

    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Transmit);
    frame = communicator.fetch_frame();
    EXPECT_EQ(frame->get_type(), Can::FrameType::SingleFrame);
    EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_len(), 3);
    EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x22, 0x20, 0x0e, 0x00, 0x00, 0x00, 0x00}));

    communicator.push_frame(std::make_shared<Can::Frame_SingleFrame>(4, std::vector<uint8_t>({0x62, 0x20, 0x0E, 0x03, 0x55, 0x55, 0x55})));
    
    EXPECT_EQ(task->is_completed(), true);
    auto res = task->get_result();
    EXPECT_EQ(res.first, "A A A A A A A A A");
    EXPECT_EQ(res.second, 0x03);
}

namespace Can {
    class TestAsyncTask : public AsyncTask {
    public:
        TestAsyncTask(Logger* logger = new NoLogger()) :
        AsyncTask(logger) {}
        void task() {
            std::shared_ptr<ServiceResponse> response;

            response = call(std::make_shared<ServiceRequest_ReadDataByIdentifier>(DataIdentifier::VIN));

            m_result = Util::vec_to_str(static_cast<ServiceResponse_ReadDataByIdentifier*>(response.get())->get_data()->get_value());
        }

        std::string get_result() {
            return m_result;
        }
    private:
        std::string m_result;
    };
}

TEST(testCommunication, testThreadedTask) {
    Can::TestAsyncTask* task = new Can::TestAsyncTask();
    Can::Communicator communicator{};
    communicator.set_task(task);
    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Transmit);

    std::shared_ptr<Can::Frame> frame = communicator.fetch_frame();

    EXPECT_EQ(frame->get_type(), Can::FrameType::SingleFrame);
    EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_len(), 3);
    EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame.get())->get_data(), std::vector<uint8_t>({0x22, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00}));

    std::vector<std::shared_ptr<Can::Frame>> frames;
    frames.push_back(std::make_shared<Can::Frame_FirstFrame>(20, std::vector<uint8_t>({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})));
    frames.push_back(std::make_shared<Can::Frame_ConsecutiveFrame>(1, std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})));
    frames.push_back(std::make_shared<Can::Frame_ConsecutiveFrame>(2, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));

    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Idle);
    for(std::shared_ptr<Can::Frame> f : frames) {
        communicator.push_frame(f);
    }
    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Idle);
    EXPECT_EQ(task->is_completed(), true);
    EXPECT_EQ(task->get_result(), "A A A A A A A A A");
}
