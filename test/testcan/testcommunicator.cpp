#include <chrono>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <vector>

#include "communicator.h"
#include "frame_all.h"
#include "gtest/gtest.h"
#include "service_all.h"
#include "task.h"
#include "util.h"

TEST(testCommunicator, testFramesToService) {
    {
        std::vector<std::shared_ptr<Can::Frame::Frame>> frames;
        frames.push_back(Can::Frame::SingleFrame::build()
                             ->len(3)
                             ->data({0x6e, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00})
                             ->build()
                             .value());
        auto maybe_response = Can::frames_to_service(frames);
        EXPECT_TRUE(maybe_response);
        std::shared_ptr<Can::ServiceResponse::ServiceResponse> response =
            maybe_response.value();
        EXPECT_EQ(response->get_type(),
                  Can::ServiceResponse::Type::WriteDataByIdentifier);
        Can::DataIdentifier id =
            std::static_pointer_cast<
                Can::ServiceResponse::WriteDataByIdentifier>(response)
                ->get_id();
        EXPECT_EQ(id, Can::DataIdentifier::VIN);
    }

    {
        std::vector<std::shared_ptr<Can::Frame::Frame>> frames;
        frames.push_back(Can::Frame::FirstFrame::build()
                             ->len(20)
                             ->data({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})
                             ->build()
                             .value());
        frames.push_back(Can::Frame::ConsecutiveFrame::build()
                             ->seq_num(1)
                             ->data({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})
                             ->build()
                             .value());
        frames.push_back(Can::Frame::ConsecutiveFrame::build()
                             ->seq_num(2)
                             ->data({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})
                             ->build()
                             .value());
        auto maybe_response = Can::frames_to_service(frames);
        EXPECT_TRUE(maybe_response);
        std::shared_ptr<Can::ServiceResponse::ServiceResponse> response =
            maybe_response.value();
        EXPECT_EQ(response->get_type(),
                  Can::ServiceResponse::Type::ReadDataByIdentifier);
        auto data = std::static_pointer_cast<
                        Can::ServiceResponse::ReadDataByIdentifier>(response)
                        ->get_data();
        EXPECT_EQ(data->get_type(), Can::DataIdentifier::VIN);
        EXPECT_EQ(data->get_value(),
                  std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20,
                                        0x41, 0x20, 0x41, 0x20, 0x41, 0x20,
                                        0x41, 0x20, 0x41, 0x20, 0x41}));
    }
}

TEST(testCommunicator, testServiceToFrames) {
    {
        auto data =
            Can::Data::build()
                ->type(Can::DataIdentifier::VIN)
                ->value({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41,
                         0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})
                ->build();
        auto maybe_request = Can::ServiceRequest::WriteDataByIdentifier::build()
                                 ->data(data.value())
                                 ->build();
        EXPECT_TRUE(maybe_request);
        auto request = maybe_request.value();
        auto maybe_frames = Can::service_to_frames(request);
        EXPECT_TRUE(maybe_frames);
        std::vector<std::shared_ptr<Can::Frame::Frame>> frames =
            maybe_frames.value();
        EXPECT_EQ(frames.size(), 3);
        EXPECT_EQ(frames[0]->get_type(), Can::Frame::Type::FirstFrame);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frames[0])
                      ->get_len(),
                  20);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frames[0])
                      ->get_data(),
                  std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(frames[1]->get_type(), Can::Frame::Type::ConsecutiveFrame);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frames[1])
                ->get_seq_num(),
            1);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frames[1])
                ->get_data(),
            std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frames[2])
                ->get_seq_num(),
            2);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frames[2])
                ->get_data(),
            std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    {
        auto maybe_request = Can::ServiceRequest::ReadDataByIdentifier::build()
                                 ->id(Can::DataIdentifier::VIN)
                                 ->build();
        EXPECT_TRUE(maybe_request);
        auto request = maybe_request.value();
        auto maybe_frames = Can::service_to_frames(request);
        EXPECT_TRUE(maybe_frames);
        std::vector<std::shared_ptr<Can::Frame::Frame>> frames =
            maybe_frames.value();
        EXPECT_EQ(frames.size(), 1);
        EXPECT_EQ(frames[0]->get_type(), Can::Frame::Type::SingleFrame);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::SingleFrame>(frames[0])
                      ->get_len(),
                  3);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::SingleFrame>(frames[0])
                ->get_data(),
            std::vector<uint8_t>({0x22, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00}));
    }
}

TEST(testCommunicator, testReceiver) {
    {
        std::vector<std::shared_ptr<Can::Frame::Frame>> frames;
        frames.push_back(Can::Frame::SingleFrame::build()
                             ->len(3)
                             ->data({0x6e, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00})
                             ->build()
                             .value());
        Can::Receiver receiver(frames[0]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Done);
        auto maybe_response = receiver.get_response();
        EXPECT_TRUE(maybe_response);
        std::shared_ptr<Can::ServiceResponse::ServiceResponse> response =
            maybe_response.value();
        EXPECT_EQ(response->get_type(),
                  Can::ServiceResponse::Type::WriteDataByIdentifier);
        Can::DataIdentifier id =
            std::static_pointer_cast<
                Can::ServiceResponse::WriteDataByIdentifier>(response)
                ->get_id();
        EXPECT_EQ(id, Can::DataIdentifier::VIN);
    }

    {
        std::vector<std::shared_ptr<Can::Frame::Frame>> frames;
        frames.push_back(Can::Frame::FirstFrame::build()
                             ->len(20)
                             ->data({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})
                             ->build()
                             .value());
        frames.push_back(Can::Frame::ConsecutiveFrame::build()
                             ->seq_num(1)
                             ->data({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})
                             ->build()
                             .value());
        frames.push_back(Can::Frame::ConsecutiveFrame::build()
                             ->seq_num(2)
                             ->data({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})
                             ->build()
                             .value());
        Can::Receiver receiver(frames[0]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Work);
        auto maybe_flow_control = receiver.fetch_frame();
        EXPECT_TRUE(maybe_flow_control);
        std::shared_ptr<Can::Frame::Frame> flow_control =
            maybe_flow_control.value();
        EXPECT_EQ(flow_control->get_type(), Can::Frame::Type::FlowControl);
        receiver.push_frame(frames[1]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Work);
        receiver.push_frame(frames[2]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Done);
        auto maybe_response = receiver.get_response();
        EXPECT_TRUE(maybe_response);
        std::shared_ptr<Can::ServiceResponse::ServiceResponse> response =
            maybe_response.value();
        EXPECT_EQ(response->get_type(),
                  Can::ServiceResponse::Type::ReadDataByIdentifier);
        auto data = std::static_pointer_cast<
                        Can::ServiceResponse::ReadDataByIdentifier>(response)
                        ->get_data();
        EXPECT_EQ(data->get_type(), Can::DataIdentifier::VIN);
        EXPECT_EQ(data->get_value(),
                  std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20,
                                        0x41, 0x20, 0x41, 0x20, 0x41, 0x20,
                                        0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    {  // Test timeout
        std::vector<std::shared_ptr<Can::Frame::Frame>> frames;
        frames.push_back(Can::Frame::FirstFrame::build()
                             ->len(20)
                             ->data({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})
                             ->build()
                             .value());
        frames.push_back(Can::Frame::ConsecutiveFrame::build()
                             ->seq_num(1)
                             ->data({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})
                             ->build()
                             .value());
        frames.push_back(Can::Frame::ConsecutiveFrame::build()
                             ->seq_num(2)
                             ->data({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})
                             ->build()
                             .value());
        Can::Receiver receiver(frames[0]);
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Work);
        auto maybe_flow_control = receiver.fetch_frame();
        EXPECT_TRUE(maybe_flow_control);
        std::shared_ptr<Can::Frame::Frame> flow_control =
            maybe_flow_control.value();
        EXPECT_EQ(flow_control->get_type(), Can::Frame::Type::FlowControl);
        receiver.TIMEOUT = std::chrono::milliseconds(50);
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        EXPECT_EQ(receiver.get_status(), Can::WorkerStatus::Error);
    }
}

TEST(testCommunicator, testTransmitter) {
    {
        auto maybe_request = Can::ServiceRequest::ReadDataByIdentifier::build()->id(Can::DataIdentifier::VIN)->build();
        EXPECT_TRUE(maybe_request);
        auto request = maybe_request.value();
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        auto maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        std::shared_ptr<Can::Frame::Frame> frame = maybe_frame.value();
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::SingleFrame);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::SingleFrame);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::SingleFrame>(frame)->get_len(),
            3);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::SingleFrame>(frame)->get_data(),
            std::vector<uint8_t>({0x22, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00}));
    }

    auto data =
        Can::Data::build()
            ->type(Can::DataIdentifier::VIN)
            ->value({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20,
                     0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})
            ->build();
    auto maybe_request = Can::ServiceRequest::WriteDataByIdentifier::build()
                             ->data(data.value())
                             ->build();
    std::shared_ptr<Can::ServiceRequest::ServiceRequest> request = maybe_request.value();
    {
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        auto maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        std::shared_ptr<Can::Frame::Frame> frame = maybe_frame.value();
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::FirstFrame);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                      ->get_len(),
                  20);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                      ->get_data(),
                  std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(
            Can::Frame::FlowControl::build()
                ->status(Can::Frame::FlowStatus::ContinueToSend)
                ->block_size(0)
                ->min_separation_time(0)
                ->build()
                .value());
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        frame = maybe_frame.value();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::ConsecutiveFrame);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_seq_num(),
            1);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_data(),
            std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        frame = maybe_frame.value();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::ConsecutiveFrame);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_seq_num(),
            2);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_data(),
            std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    {  // Test BlockSize = 1
        Can::Transmitter transmitter(request);
        transmitter.TIMEOUT = std::chrono::milliseconds(50);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        auto maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        std::shared_ptr<Can::Frame::Frame> frame = maybe_frame.value();
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::FirstFrame);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                      ->get_len(),
                  20);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                      ->get_data(),
                  std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        transmitter.push_frame(
            Can::Frame::FlowControl::build()
                ->status(Can::Frame::FlowStatus::ContinueToSend)
                ->block_size(1)
                ->min_separation_time(0)
                ->build()
                .value());
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        frame = maybe_frame.value();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::ConsecutiveFrame);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_seq_num(),
            1);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_data(),
            std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        maybe_frame = transmitter.fetch_frame();
        EXPECT_FALSE(maybe_frame);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        transmitter.push_frame(
            Can::Frame::FlowControl::build()
                ->status(Can::Frame::FlowStatus::ContinueToSend)
                ->block_size(1)
                ->min_separation_time(0)
                ->build()
                .value());
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        frame = maybe_frame.value();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_seq_num(),
            2);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_data(),
            std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    {  // Test Timeout
        Can::Transmitter transmitter(request);
        transmitter.TIMEOUT = std::chrono::milliseconds(50);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        auto maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        std::shared_ptr<Can::Frame::Frame> frame = maybe_frame.value();
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::FirstFrame);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                      ->get_len(),
                  20);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                      ->get_data(),
                  std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(
            Can::Frame::FlowControl::build()
                ->status(Can::Frame::FlowStatus::ContinueToSend)
                ->block_size(1)
                ->min_separation_time(0)
                ->build()
                .value());
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        frame = maybe_frame.value();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::ConsecutiveFrame);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_seq_num(),
            1);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_data(),
            std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        maybe_frame = transmitter.fetch_frame();
        EXPECT_FALSE(maybe_frame);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::this_thread::sleep_for(std::chrono::milliseconds(60));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Error);
    }

    {  // Test FlowControl Repeat type
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        auto maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        std::shared_ptr<Can::Frame::Frame> frame = maybe_frame.value();
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::FirstFrame);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                      ->get_len(),
                  20);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                      ->get_data(),
                  std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(
            Can::Frame::FlowControl::build()
                ->status(Can::Frame::FlowStatus::WaitForAnotherFlowControlMessageBeforeContinuing)
                ->block_size(0)
                ->min_separation_time(0)
                ->build()
                .value());
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        maybe_frame = transmitter.fetch_frame();
        EXPECT_FALSE(maybe_frame);
        transmitter.push_frame(
            Can::Frame::FlowControl::build()
                ->status(Can::Frame::FlowStatus::ContinueToSend)
                ->block_size(0)
                ->min_separation_time(0)
                ->build()
                .value());
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        frame = maybe_frame.value();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::ConsecutiveFrame);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_seq_num(),
            1);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_data(),
            std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        frame = maybe_frame.value();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::ConsecutiveFrame);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_seq_num(),
            2);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_data(),
            std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    {  // Test Minimum separation time
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        auto maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        std::shared_ptr<Can::Frame::Frame> frame = maybe_frame.value();
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::FirstFrame);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                      ->get_len(),
                  20);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                      ->get_data(),
                  std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(
            Can::Frame::FlowControl::build()
                ->status(Can::Frame::FlowStatus::ContinueToSend)
                ->block_size(1)
                ->min_separation_time(30)
                ->build()
                .value());
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        maybe_frame = transmitter.fetch_frame();
        EXPECT_FALSE(maybe_frame);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        frame = maybe_frame.value();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::ConsecutiveFrame);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_seq_num(),
            1);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_data(),
            std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        transmitter.push_frame(
            Can::Frame::FlowControl::build()
                ->status(Can::Frame::FlowStatus::ContinueToSend)
                ->block_size(1)
                ->min_separation_time(0)
                ->build()
                .value());
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        frame = maybe_frame.value();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_seq_num(),
            2);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_data(),
            std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }

    {
        Can::Transmitter transmitter(request);
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        auto maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        std::shared_ptr<Can::Frame::Frame> frame = maybe_frame.value();
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::FirstFrame);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                      ->get_len(),
                  20);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FirstFrame>(frame)
                      ->get_data(),
                  std::vector<uint8_t>({0x2e, 0xf1, 0x90, 0x41, 0x20, 0x41}));
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        transmitter.push_frame(
            Can::Frame::FlowControl::build()
                ->status(Can::Frame::FlowStatus::ContinueToSend)
                ->block_size(0)
                ->min_separation_time(30)
                ->build()
                .value());
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        maybe_frame = transmitter.fetch_frame();
        EXPECT_FALSE(maybe_frame);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        frame = maybe_frame.value();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Work);
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::ConsecutiveFrame);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_seq_num(),
            1);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_data(),
            std::vector<uint8_t>({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20}));
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        maybe_frame = transmitter.fetch_frame();
        EXPECT_FALSE(maybe_frame);
        std::this_thread::sleep_for(std::chrono::milliseconds(40));
        maybe_frame = transmitter.fetch_frame();
        EXPECT_TRUE(maybe_frame);
        frame = maybe_frame.value();
        EXPECT_EQ(transmitter.get_status(), Can::WorkerStatus::Done);
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::ConsecutiveFrame);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_seq_num(),
            2);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_data(),
            std::vector<uint8_t>({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41}));
    }
}

namespace Can {

class TestTask_Read : public Task {
public:
    optional<std::shared_ptr<ServiceRequest::ServiceRequest>> fetch_request() {
        m_step++;
        switch (m_step - 1) {
            case 0:
                return ServiceRequest::ReadDataByIdentifier::build()
                    ->id(DataIdentifier::VIN)
                    ->build();
            default:
                m_step--;
                return {};
        }
    }

    void push_response(
        std::shared_ptr<ServiceResponse::ServiceResponse> response) {
        m_step++;
        switch (m_step - 1) {
            case 1:
                m_result = Util::vec_to_str(
                    std::static_pointer_cast<
                        ServiceResponse::ReadDataByIdentifier>(response)
                        ->get_data()
                        ->get_value());
                break;
            default:
                m_step--;
                return;
        }
    }

    std::string get_result() { return m_result; }

    bool is_completed() { return m_step == 2; }

private:
    int m_step = 0;
    std::string m_result;
};

class TestTask_DoubleRead : public Task {
public:
    optional<std::shared_ptr<ServiceRequest::ServiceRequest>> fetch_request() {
        m_step++;
        switch (m_step - 1) {
            case 0:
                return ServiceRequest::ReadDataByIdentifier::build()
                    ->id(DataIdentifier::VIN)
                    ->build();
            case 2:
                return ServiceRequest::ReadDataByIdentifier::build()
                    ->id(DataIdentifier::UPASystemType)
                    ->build();
            default:
                m_step--;
                return {};
        }
    }

    void push_response(
        std::shared_ptr<ServiceResponse::ServiceResponse> response) {
        m_step++;
        switch (m_step - 1) {
            case 1:
                m_result_VIN = Util::vec_to_str(
                    std::static_pointer_cast<
                        ServiceResponse::ReadDataByIdentifier>(response)
                        ->get_data()
                        ->get_value());
                break;
            case 3:
                m_result_UPASystemType =
                    std::static_pointer_cast<
                        ServiceResponse::ReadDataByIdentifier>(response)
                        ->get_data()
                        ->get_value()[0];
                break;
            default:
                m_step--;
                return;
        }
    }

    std::pair<std::string, uint8_t> get_result() {
        return std::make_pair(m_result_VIN, m_result_UPASystemType);
    }

    bool is_completed() { return m_step == 4; }

private:
    int m_step = 0;
    std::string m_result_VIN;
    uint8_t m_result_UPASystemType;
};

}  // namespace Can

TEST(testCommunicator, testTaskRead) {
    auto task = std::make_shared<Can::TestTask_Read>();
    Can::Communicator communicator{};
    communicator.set_task(task);
    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Transmit);
    std::shared_ptr<Can::Frame::Frame> frame = communicator.fetch_frame().value();
    EXPECT_EQ(frame->get_type(), Can::Frame::Type::SingleFrame);
    EXPECT_EQ(std::static_pointer_cast<Can::Frame::SingleFrame>(frame)
                  ->get_len(),
              3);
    EXPECT_EQ(std::static_pointer_cast<Can::Frame::SingleFrame>(frame)
                  ->get_data(),
              std::vector<uint8_t>({0x22, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00}));

    std::vector<std::shared_ptr<Can::Frame::Frame>> frames;

    frames.push_back(Can::Frame::FirstFrame::build()
                         ->len(20)
                         ->data({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})
                         ->build()
                         .value());
    frames.push_back(Can::Frame::ConsecutiveFrame::build()
                         ->seq_num(1)
                         ->data({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})
                         ->build()
                         .value());
    frames.push_back(Can::Frame::ConsecutiveFrame::build()
                         ->seq_num(2)
                         ->data({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})
                         ->build()
                         .value());

    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Idle);
    for (std::shared_ptr<Can::Frame::Frame> f : frames) {
        communicator.push_frame(f);
    }
    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Idle);
    EXPECT_EQ(task->is_completed(), true);
    EXPECT_EQ(task->get_result(), "A A A A A A A A A");
}

TEST(testCommunicator, testTaskWriteRead) {
    auto task = std::make_shared<Can::TestTask_DoubleRead>();
    Can::Communicator communicator{};
    communicator.set_task(task);
    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Transmit);

    std::shared_ptr<Can::Frame::Frame> frame = communicator.fetch_frame().value();
    EXPECT_EQ(frame->get_type(), Can::Frame::Type::SingleFrame);
    EXPECT_EQ(std::static_pointer_cast<Can::Frame::SingleFrame>(frame)
                  ->get_len(),
              3);
    EXPECT_EQ(std::static_pointer_cast<Can::Frame::SingleFrame>(frame)
                  ->get_data(),
              std::vector<uint8_t>({0x22, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00}));

    std::vector<std::shared_ptr<Can::Frame::Frame>> frames;

    frames.push_back(Can::Frame::FirstFrame::build()
                         ->len(20)
                         ->data({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})
                         ->build()
                         .value());
    frames.push_back(Can::Frame::ConsecutiveFrame::build()
                         ->seq_num(1)
                         ->data({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})
                         ->build()
                         .value());
    frames.push_back(Can::Frame::ConsecutiveFrame::build()
                         ->seq_num(2)
                         ->data({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})
                         ->build()
                         .value());

    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Idle);
    auto no_frame = communicator.fetch_frame();
    ASSERT_FALSE(no_frame);
    for (std::shared_ptr<Can::Frame::Frame> f : frames) {
        communicator.push_frame(f);
    }

    EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Transmit);
    frame = communicator.fetch_frame().value();
    EXPECT_EQ(frame->get_type(), Can::Frame::Type::SingleFrame);
    EXPECT_EQ(std::static_pointer_cast<Can::Frame::SingleFrame>(frame)
                  ->get_len(),
              3);
    EXPECT_EQ(std::static_pointer_cast<Can::Frame::SingleFrame>(frame)
                  ->get_data(),
              std::vector<uint8_t>({0x22, 0x20, 0x0e, 0x00, 0x00, 0x00, 0x00}));

    communicator.push_frame(
        Can::Frame::SingleFrame::build()
            ->len(4)
            ->data({0x62, 0x20, 0x0E, 0x03, 0x55, 0x55, 0x55})
            ->build()
            .value());

    EXPECT_EQ(task->is_completed(), true);
    auto res = task->get_result();
    EXPECT_EQ(res.first, "A A A A A A A A A");
    EXPECT_EQ(res.second, 0x03);
}

namespace Can {
class TestAsyncTask : public AsyncTask {
public:
    TestAsyncTask(Logger* logger = new NoLogger()) : AsyncTask(logger) {}
    void task() {
        std::shared_ptr<ServiceResponse::ServiceResponse> response;

        response = call(ServiceRequest::ReadDataByIdentifier::build()
                            ->id(DataIdentifier::VIN)
                            ->build()
                            .value());

        m_result = Util::vec_to_str(
            std::static_pointer_cast<ServiceResponse::ReadDataByIdentifier>(
                response)
                ->get_data()
                ->get_value());
    }

    std::string get_result() { return m_result; }

private:
    std::string m_result;
};
}  // namespace Can

// TEST(testCommunication, testThreadedTask) {
//     auto task = std::make_shared<Can::TestAsyncTask>();
//     Can::Communicator communicator{};
//     communicator.set_task(task);
//     EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Transmit);

//     std::shared_ptr<Can::Frame::Frame> frame = communicator.fetch_frame().value();

//     EXPECT_EQ(frame->get_type(), Can::Frame::Type::SingleFrame);
//     EXPECT_EQ(
//         std::static_pointer_cast<Can::Frame::SingleFrame>(frame)->get_len(), 3);
//     EXPECT_EQ(
//         std::static_pointer_cast<Can::Frame::SingleFrame>(frame)->get_data(),
//         std::vector<uint8_t>({0x22, 0xf1, 0x90, 0x00, 0x00, 0x00, 0x00}));

//     std::vector<std::shared_ptr<Can::Frame::Frame>> frames;
//     frames.push_back(Can::Frame::FirstFrame::build()
//                          ->len(20)
//                          ->data({0x62, 0xf1, 0x90, 0x41, 0x20, 0x41})
//                          ->build()
//                          .value());
//     frames.push_back(Can::Frame::ConsecutiveFrame::build()
//                          ->seq_num(1)
//                          ->data({0x20, 0x41, 0x20, 0x41, 0x20, 0x41, 0x20})
//                          ->build()
//                          .value());
//     frames.push_back(Can::Frame::ConsecutiveFrame::build()
//                          ->seq_num(2)
//                          ->data({0x41, 0x20, 0x41, 0x20, 0x41, 0x20, 0x41})
//                          ->build()
//                          .value());

//     EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Idle);
//     for (std::shared_ptr<Can::Frame::Frame> f : frames) {
//         communicator.push_frame(f);
//     }
//     EXPECT_EQ(communicator.get_status(), Can::CommunicatorStatus::Idle);
//     EXPECT_EQ(task->is_completed(), true);
//     EXPECT_EQ(task->get_result(), "A A A A A A A A A");
// }
