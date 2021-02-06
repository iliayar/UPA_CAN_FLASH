#include "gtest/gtest.h"
#include "frame.h"
#include "communicator.h"

#include <vector>
#include <chrono>
#include <thread>

TEST(testCommunicator, testFramesToService)
{
    {
        std::vector<Can::Frame*> frames;
        frames.push_back(new Can::Frame_SingleFrame(3, std::vector<uint8_t>({0x6e, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00})));
        Can::ServiceResponse* response = Can::frames_to_service(frames);
        EXPECT_EQ(response->get_type(), Can::ServiceResponseType::WriteDataByIdentifier);
        Can::DataIdentifier id = static_cast<Can::ServiceResponse_WriteDataByIdentifier*>(response)->get_id();
        EXPECT_EQ(id, Can::DataIdentifier::VIN);
    }

    {
        std::vector<Can::Frame*> frames;
        frames.push_back(new Can::Frame_FirstFrame(20, std::vector<uint8_t>({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})));
        frames.push_back(new Can::Frame_ConsecutiveFrame(1, std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})));
        frames.push_back(new Can::Frame_ConsecutiveFrame(2, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::ServiceResponse* response = Can::frames_to_service(frames);
        EXPECT_EQ(response->get_type(), Can::ServiceResponseType::ReadDataByIdentifier);
        Can::Data* data = static_cast<Can::ServiceResponse_ReadDataByIdentifier*>(response)->get_data();
        EXPECT_EQ(data->get_type(), Can::DataIdentifier::VIN);
        EXPECT_EQ(data->get_value(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }
}

TEST(testCommunicator, testServiceToFrames)
{
    {
        Can::ServiceRequest* request = new Can::ServiceRequest_WriteDataByIdentifier(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        std::vector<Can::Frame*> frames = Can::service_to_frames(request);
        EXPECT_EQ(frames.size(), 3);
        EXPECT_EQ(frames[0]->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frames[0])->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frames[0])->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(frames[1]->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frames[1])->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frames[1])->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frames[2])->get_seq_num(), 2);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frames[2])->get_data(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    {
        Can::ServiceRequest* request = new Can::ServiceRequest_ReadDataByIdentifier(Can::DataIdentifier::VIN);
        std::vector<Can::Frame*> frames = Can::service_to_frames(request);
        EXPECT_EQ(frames.size(), 1);
        EXPECT_EQ(frames[0]->get_type(), Can::FrameType::SingleFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frames[0])->get_len(), 3);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frames[0])->get_data(), std::vector<uint8_t>({0x22, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00}));
    }
}

TEST(testCommunicator, testReceiver)
{
    {
        std::vector<Can::Frame*> frames;
        frames.push_back(new Can::Frame_SingleFrame(3, std::vector<uint8_t>({0x6e, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00})));
        Can::Receiver receiver(frames[0]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Done);
        Can::ServiceResponse* response = receiver.get_response();
        EXPECT_EQ(response->get_type(), Can::ServiceResponseType::WriteDataByIdentifier);
        Can::DataIdentifier id = static_cast<Can::ServiceResponse_WriteDataByIdentifier*>(response)->get_id();
        EXPECT_EQ(id, Can::DataIdentifier::VIN);
    }

    {
        std::vector<Can::Frame*> frames;
        frames.push_back(new Can::Frame_FirstFrame(20, std::vector<uint8_t>({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})));
        frames.push_back(new Can::Frame_ConsecutiveFrame(1, std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})));
        frames.push_back(new Can::Frame_ConsecutiveFrame(2, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Receiver receiver(frames[0]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Work);
        Can::Frame* flow_control = receiver.fetch_frame();
        EXPECT_EQ(flow_control->get_type(), Can::FrameType::FlowControl);
        receiver.push_frame(frames[1]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Work);
        receiver.push_frame(frames[2]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Done);
        Can::ServiceResponse* response = receiver.get_response();
        EXPECT_EQ(response->get_type(), Can::ServiceResponseType::ReadDataByIdentifier);
        Can::Data* data = static_cast<Can::ServiceResponse_ReadDataByIdentifier*>(response)->get_data();
        EXPECT_EQ(data->get_type(), Can::DataIdentifier::VIN);
        EXPECT_EQ(data->get_value(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    { // Test timeout
        std::vector<Can::Frame*> frames;
        frames.push_back(new Can::Frame_FirstFrame(20, std::vector<uint8_t>({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})));
        frames.push_back(new Can::Frame_ConsecutiveFrame(1, std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})));
        frames.push_back(new Can::Frame_ConsecutiveFrame(2, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Receiver receiver(frames[0]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Work);
        Can::Frame* flow_control = receiver.fetch_frame();
        EXPECT_EQ(flow_control->get_type(), Can::FrameType::FlowControl);
        receiver.TIMEOUT = std::chrono::milliseconds(50);
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Error);
    }
}


TEST(testCommunicator, testTransmitter)
{
    {
        Can::ServiceRequest* request = new Can::ServiceRequest_ReadDataByIdentifier(Can::DataIdentifier::VIN);
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        Can::Frame* frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::SingleFrame);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(frame->get_type(), Can::FrameType::SingleFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_len(), 3);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_data(), std::vector<uint8_t>({0x22, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00}));
    }

    {
        Can::ServiceRequest* request = new Can::ServiceRequest_WriteDataByIdentifier(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        Can::Frame* frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(new Can::Frame_FlowControl(Can::FlowStatus::ContinueToSend, 0, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_seq_num(), 2);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_data(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    { // Test BlockSize = 1
        Can::ServiceRequest* request = new Can::ServiceRequest_WriteDataByIdentifier(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Transmitter transmitter(request);
        transmitter.TIMEOUT = std::chrono::milliseconds(50);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        Can::Frame* frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        transmitter.push_frame(new Can::Frame_FlowControl(Can::FlowStatus::ContinueToSend, 1, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        transmitter.push_frame(new Can::Frame_FlowControl(Can::FlowStatus::ContinueToSend, 1, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_seq_num(), 2);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_data(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    { // Test Timeout
        Can::ServiceRequest* request = new Can::ServiceRequest_WriteDataByIdentifier(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Transmitter transmitter(request);
        transmitter.TIMEOUT = std::chrono::milliseconds(50);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        Can::Frame* frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(new Can::Frame_FlowControl(Can::FlowStatus::ContinueToSend, 1, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Error);
    }

    { // Test FlowControl Repeat type
        Can::ServiceRequest* request = new Can::ServiceRequest_WriteDataByIdentifier(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        Can::Frame* frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(new Can::Frame_FlowControl(Can::FlowStatus::WaitForAnotherFlowControlMessageBeforeContinuing, 0, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(frame, nullptr);
        transmitter.push_frame(new Can::Frame_FlowControl(Can::FlowStatus::ContinueToSend, 0, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_seq_num(), 2);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_data(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    { // Test Minimum separation time
        Can::ServiceRequest* request = new Can::ServiceRequest_WriteDataByIdentifier(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        Can::Frame* frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(new Can::Frame_FlowControl(Can::FlowStatus::ContinueToSend, 1, 30));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(frame, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        frame = transmitter.fetch_frame();
        EXPECT_NE(frame, nullptr);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        transmitter.push_frame(new Can::Frame_FlowControl(Can::FlowStatus::ContinueToSend, 1, 0));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        frame = transmitter.fetch_frame();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_seq_num(), 2);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_data(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    {
        Can::ServiceRequest* request = new Can::ServiceRequest_WriteDataByIdentifier(new Can::Data(Can::DataIdentifier::VIN, std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})));
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        Can::Frame* frame = transmitter.fetch_frame();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_len(), 20);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_data(), std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(new Can::Frame_FlowControl(Can::FlowStatus::ContinueToSend, 0, 30));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(frame, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        frame = transmitter.fetch_frame();
        EXPECT_NE(frame, nullptr);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_seq_num(), 1);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_data(), std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        frame = transmitter.fetch_frame();
        EXPECT_EQ(frame, nullptr);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        frame = transmitter.fetch_frame();
        EXPECT_NE(frame, nullptr);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_seq_num(), 2);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_data(), std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }
}
