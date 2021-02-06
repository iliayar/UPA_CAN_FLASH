#include "gtest/gtest.h"
#include "frame.h"
#include "communicator.h"

#include <vector>

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
