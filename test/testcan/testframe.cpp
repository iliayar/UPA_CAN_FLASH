#include <vector>

#include "frame_all.h"
#include "gtest/gtest.h"

TEST(testFrame, testFrameFactory) {
    {
        std::shared_ptr<Can::Frame::Frame> frame;
        {
            auto maybe_frame = Can::Frame::Factory({0x02, 0x13, 0x37, 0x00,
                                                    0x00, 0x00, 0x00, 0x00})
                                   .get();
            EXPECT_TRUE(maybe_frame);
            frame = maybe_frame.value();
        }
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::SingleFrame);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::SingleFrame>(frame)->get_len(),
            2);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::SingleFrame>(frame)
                ->get_data(),
            std::vector<uint8_t>({0x13, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00}));
    }

    {
        std::shared_ptr<Can::Frame::Frame> frame;
        {
            auto maybe_frame = Can::Frame::Factory({0x14, 0x02, 0x12, 0x00,
                                                    0x34, 0x00, 0x00, 0x00})
                                   .get();
            EXPECT_TRUE(maybe_frame);
            frame = maybe_frame.value();
        }
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::FirstFrame);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::FirstFrame>(frame)->get_len(),
            0x402);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::FirstFrame>(frame)->get_data(),
            std::vector<uint8_t>({0x12, 0x00, 0x34, 0x00, 0x00, 0x00}));
    }

    {
        std::shared_ptr<Can::Frame::Frame> frame;
        {
            auto maybe_frame = Can::Frame::Factory({0x2a, 0x00, 0x13, 0x37,
                                                    0x42, 0x54, 0x00, 0x00})
                                   .get();
            EXPECT_TRUE(maybe_frame);
            frame = maybe_frame.value();
        }
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::ConsecutiveFrame);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                      ->get_seq_num(),
                  10);
        EXPECT_EQ(
            std::static_pointer_cast<Can::Frame::ConsecutiveFrame>(frame)
                ->get_data(),
            std::vector<uint8_t>({0x00, 0x13, 0x37, 0x42, 0x54, 0x00, 0x00}));
    }

    {
        std::shared_ptr<Can::Frame::Frame> frame;
        {
            auto maybe_frame = Can::Frame::Factory({0x32, 0x10, 0x05, 0x00,
                                                    0x00, 0x00, 0x00, 0x00})
                                   .get();
            EXPECT_TRUE(maybe_frame);
            frame = maybe_frame.value();
        }
        EXPECT_EQ(frame->get_type(), Can::Frame::Type::FlowControl);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FlowControl>(frame)
                      ->get_status(),
                  Can::Frame::FlowStatus::OverflowAbortTransmission);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FlowControl>(frame)
                      ->get_block_size(),
                  16);
        EXPECT_EQ(std::static_pointer_cast<Can::Frame::FlowControl>(frame)
                      ->get_min_separation_time(),
                  5);
    }
}

TEST(testFrame, testFrameWrite) {
    {
        auto maybe_frame = Can::Frame::SingleFrame::build()
                               ->len(2)
                               ->data({0x13, 0x37})
                               ->build();
        EXPECT_TRUE(maybe_frame);
        auto frame = maybe_frame.value();
        auto maybe_payload = frame->dump();
        EXPECT_TRUE(maybe_payload);
        auto payload = maybe_payload.value();
        std::vector<uint8_t> res = {0x02, 0x13, 0x37, 0x00,
                                    0x00, 0x00, 0x00, 0x00};
        EXPECT_EQ(payload, res);
    }

    {
        auto maybe_frame = Can::Frame::FirstFrame::build()
                               ->len(0x402)
                               ->data({0x12, 0x00, 0x34, 0x00, 0x00, 0x00})
                               ->build();
        EXPECT_TRUE(maybe_frame);
        auto frame = maybe_frame.value();
        auto maybe_payload = frame->dump();
        EXPECT_TRUE(maybe_payload);
        auto payload = maybe_payload.value();
        std::vector<uint8_t> res = {0x14, 0x02, 0x12, 0x00, 0x34, 0x00, 0x00,
        0x00}; EXPECT_EQ(payload, res);
    }

    {
        auto maybe_frame =
            Can::Frame::ConsecutiveFrame::build()
                ->seq_num(10)
                ->data({0x00, 0x13, 0x37, 0x42, 0x54, 0x00, 0x00})
                ->build();
        EXPECT_TRUE(maybe_frame);
        auto frame = maybe_frame.value();
        auto maybe_payload = frame->dump();
        EXPECT_TRUE(maybe_payload);
        auto payload = maybe_payload.value();
        std::vector<uint8_t> res = {0x2a, 0x00, 0x13, 0x37, 0x42, 0x54, 0x00,
        0x00}; EXPECT_EQ(payload, res);
    }

    {
        auto maybe_frame =
            Can::Frame::FlowControl::build()
                ->status(Can::Frame::FlowStatus::OverflowAbortTransmission)
                ->block_size(16)
                ->min_separation_time(5)
                ->build();
        EXPECT_TRUE(maybe_frame);
        auto frame = maybe_frame.value();
        auto maybe_payload = frame->dump();
        EXPECT_TRUE(maybe_payload);
        auto payload = maybe_payload.value();
        std::vector<uint8_t> res = {0x32, 0x10, 0x05, 0x00, 0x00, 0x00, 0x00,
        0x00}; EXPECT_EQ(payload, res);
    }
}
