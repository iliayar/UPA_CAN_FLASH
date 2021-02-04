#include "gtest/gtest.h"
#include "can.h"
#include "frame.h"

#include <vector>


TEST(testReader, testArrayReader)
{
    {
        Can::Reader reader({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
        std::vector<uint8_t> res = {0x00, 0x00};
        std::vector<uint8_t> res_real = reader.read(0, 16);
        EXPECT_EQ(res, res_real);
    }

    {
        Can::Reader reader({0x13, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
        std::vector<uint8_t> res = {0x13, 0x37};
        std::vector<uint8_t> res_real = reader.read(0, 16);
        EXPECT_EQ(res, res_real);
    }

    {
        Can::Reader reader({0x13, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00});
        std::vector<uint8_t> res = {0x33, 0x70};
        std::vector<uint8_t> res_real = reader.read(4, 12);
        EXPECT_EQ(res, res_real);
    }

    {
        Can::Reader reader({0x13, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
        std::vector<uint8_t> res = {0x99, 0xb8};
        std::vector<uint8_t> res_real = reader.read(3, 16);
        EXPECT_EQ(res, res_real);
    }

    {
        Can::Reader reader({0x13, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00});
        std::vector<uint8_t> res = {0x6e, 0x80};
        std::vector<uint8_t> res_real = reader.read(9, 13);
        EXPECT_EQ(res, res_real);
    }
}

TEST(testWriter, testArrayWriter)
{
    {
        std::vector<uint8_t> payload(8, 0);
        Can::Writer writer(payload);
        std::vector<uint8_t> res = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writer.write({0x00, 0x00}, 0, 16);
        EXPECT_EQ(res, payload);
    }

    {
        std::vector<uint8_t> payload(8, 0);
        Can::Writer writer(payload);
        std::vector<uint8_t> res = {0x13, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writer.write({0x13, 0x37}, 0, 16);
        EXPECT_EQ(res, payload);
    }

    {
        std::vector<uint8_t> payload(8, 0);
        Can::Writer writer(payload);
        std::vector<uint8_t> res = {0x03, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writer.write({0x33, 0x72}, 4, 12);
        EXPECT_EQ(res, payload);
    }

    {
        std::vector<uint8_t> payload(8, 0);
        Can::Writer writer(payload);
        std::vector<uint8_t> res = {0x13, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writer.write({0x99, 0xb8}, 3, 16);
        EXPECT_EQ(res, payload);
    }

    {
        std::vector<uint8_t> payload(8, 0);
        Can::Writer writer(payload);
        std::vector<uint8_t> res = {0x00, 0x37, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};
        writer.write({0x6e, 0x84}, 9, 13);
        EXPECT_EQ(res, payload);
    }
}

TEST(testReader, testNumbersReader)
{
    {
        Can::Reader reader({0x13, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x12});
        EXPECT_EQ(static_cast<uint8_t> (0x6e), reader.read_8(9, 8));
        EXPECT_EQ(static_cast<uint16_t>(0x6e84), reader.read_16(9, 16));
        EXPECT_EQ(static_cast<uint32_t>(0x6e840000), reader.read_32(9, 32));
        EXPECT_EQ(static_cast<uint64_t>(0x1337420000000012), reader.read_64(0, 64));
    }

    {
        Can::Reader reader({0x00, 0x37, 0x42, 0x12, 0x34, 0x00, 0x00, 0x00});
        EXPECT_EQ(static_cast<uint8_t>(0x03), reader.read_8(4, 8));
        EXPECT_EQ(static_cast<uint16_t>(0x0374), reader.read_16(4, 16));
        EXPECT_EQ(static_cast<uint32_t>(0x03742123), reader.read_32(4, 32));
        EXPECT_EQ(static_cast<uint64_t>(0x0000001234000000), reader.read_64(24, 40));
    }

    {
        Can::Reader reader({0x13, 0x00, 0x42, 0x53, 0x00, 0x12, 0x34, 0x56});
        EXPECT_EQ(static_cast<uint8_t>(0x30), reader.read_8(4, 8));
        EXPECT_EQ(static_cast<uint16_t>(0x3004), reader.read_16(4, 16));
        EXPECT_EQ(static_cast<uint32_t>(0x30042530), reader.read_32(4, 32));
        EXPECT_EQ(static_cast<uint64_t>(0x0000425300123456), reader.read_64(16, 48));
    }
}

TEST(testWriter, testNumberWriter)
{
    {
        std::vector<uint8_t> payload(8, 0);
        Can::Writer writer(payload);
        std::vector<uint8_t> res;
        res = {0x00, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writer.write_8(0x6e, 9, 8);
        EXPECT_EQ(res, payload);

        res = {0x00, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00};
        writer.write_16(0x6e84, 9, 16);
        EXPECT_EQ(res, payload);

        res = {0x00, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00};
        writer.write_32(0x6e840000, 9, 32);
        EXPECT_EQ(res, payload);

        res = {0x13, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x12};
        writer.write_64(0x1337420000000012, 0, 64);
        EXPECT_EQ(res, payload);
    } 

    {
        std::vector<uint8_t> payload(8, 0);
        Can::Writer writer(payload);
        std::vector<uint8_t> res;
        res = {0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writer.write_8(0x03, 4, 8);
        EXPECT_EQ(res, payload);

        res = {0x00, 0x37, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};
        writer.write_16(0x0374, 4, 16);
        EXPECT_EQ(res, payload);

        res = {0x00, 0x37, 0x42, 0x12, 0x30, 0x00, 0x00, 0x00};
        writer.write_32(0x03742123, 4, 32);
        EXPECT_EQ(res, payload);

        res = {0x00, 0x37, 0x42, 0x12, 0x34, 0x00, 0x00, 0x00};
        writer.write_64(0x0000001234000000, 24, 40);
        EXPECT_EQ(res, payload);
    } 
    
    {
        std::vector<uint8_t> payload(8, 0);
        Can::Writer writer(payload);
        std::vector<uint8_t> res;
        res = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writer.write_8(0x30, 4, 8);
        EXPECT_EQ(res, payload);

        res = {0x03, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};
        writer.write_16(0x3004, 4, 16);
        EXPECT_EQ(res, payload);

        res = {0x03, 0x00, 0x42, 0x53, 0x00, 0x00, 0x00, 0x00};
        writer.write_32(0x30042530, 4, 32);
        EXPECT_EQ(res, payload);

        res = {0x03, 0x00, 0x42, 0x53, 0x00, 0x12, 0x34, 0x56};
        writer.write_64(0x0000425300123456, 16, 48);
        EXPECT_EQ(res, payload);
    } 
}

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
