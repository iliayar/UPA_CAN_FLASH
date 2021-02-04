#include "gtest/gtest.h"
#include "frame.h"

#include <vector>

TEST(testFrame, testFrameFactory)
{
    {
        Can::Frame* frame = Can::FrameFactory({0x02, 0x13, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00}).get();
        EXPECT_EQ(frame->get_type(), Can::FrameType::SingleFrame);
        EXPECT_EQ(static_cast<Can::Frame_SingleFrame*>(frame)->get_len(), 2);
        EXPECT_EQ(static_cast<Can::Frame_SingleFrame*>(frame)->get_data(), std::vector<uint8_t>({0x13, 0x37}));
    }

    {
        Can::Frame* frame = Can::FrameFactory({0x14, 0x02, 0x12, 0x00, 0x34, 0x00, 0x00, 0x00}).get();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FirstFrame);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_len(), 0x402);
        EXPECT_EQ(static_cast<Can::Frame_FirstFrame*>(frame)->get_data(), std::vector<uint8_t>({0x12, 0x00, 0x34, 0x00, 0x00, 0x00}));
    }

    {
        Can::Frame* frame = Can::FrameFactory({0x2a, 0x00, 0x13, 0x37, 0x42, 0x54, 0x00, 0x00}).get();
        EXPECT_EQ(frame->get_type(), Can::FrameType::ConsecutiveFrame);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_seq_num(), 10);
        EXPECT_EQ(static_cast<Can::Frame_ConsecutiveFrame*>(frame)->get_data(), std::vector<uint8_t>({0x00, 0x13, 0x37, 0x42, 0x54, 0x00, 0x00}));
    }

    {
        Can::Frame* frame = Can::FrameFactory({0x32, 0x10, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00}).get();
        EXPECT_EQ(frame->get_type(), Can::FrameType::FlowControl);
        EXPECT_EQ(static_cast<Can::Frame_FlowControl*>(frame)->get_status(), Can::FlowStatus::OverflowAbortTransmission);
        EXPECT_EQ(static_cast<Can::Frame_FlowControl*>(frame)->get_block_size(), 16);
        EXPECT_EQ(static_cast<Can::Frame_FlowControl*>(frame)->get_min_separation_time(), 5);
    }
}

TEST(testFrame, testFrameWrite)
{
    {
        Can::Frame_SingleFrame frame(2, std::vector<uint8_t>({0x13, 0x37}));
        std::vector<uint8_t> payload(8, 0);
        std::vector<uint8_t> res = {0x02, 0x13, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00};
        frame.write(Can::Writer(payload));
        EXPECT_EQ(payload, res);
    }

    {
        Can::Frame_FirstFrame frame(0x402, std::vector<uint8_t>({0x12, 0x00, 0x34, 0x00, 0x00, 0x00}));
        std::vector<uint8_t> payload(8, 0);
        std::vector<uint8_t> res = {0x14, 0x02, 0x12, 0x00, 0x34, 0x00, 0x00, 0x00};
        frame.write(Can::Writer(payload));
        EXPECT_EQ(payload, res);
    }

    {
        Can::Frame_ConsecutiveFrame frame(10, std::vector<uint8_t>({0x00, 0x13, 0x37, 0x42, 0x54, 0x00, 0x00}));
        std::vector<uint8_t> payload(8, 0);
        std::vector<uint8_t> res = {0x2a, 0x00, 0x13, 0x37, 0x42, 0x54, 0x00, 0x00};
        frame.write(Can::Writer(payload));
        EXPECT_EQ(payload, res);
    }

    {
        Can::Frame_FlowControl frame(Can::FlowStatus::OverflowAbortTransmission, 16, 5);
        std::vector<uint8_t> payload(8, 0);
        std::vector<uint8_t> res = {0x32, 0x10, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00};
        frame.write(Can::Writer(payload));
        EXPECT_EQ(payload, res);
    }
}
