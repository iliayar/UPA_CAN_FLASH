#include "gtest/gtest.h"
#include "bytes.h"

#include <vector>


TEST(testReader, testArrayReader)
{
    {
        Util::Reader reader({0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
        std::vector<uint8_t> res = {0x00, 0x00};
        auto maybe_res_real = reader.read(16);
        EXPECT_TRUE(maybe_res_real);
        std::vector<uint8_t> res_real = maybe_res_real.value();
        EXPECT_EQ(res, res_real);
    }

    {
        Util::Reader reader({0x13, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
        std::vector<uint8_t> res = {0x13, 0x37};
        auto maybe_res_real = reader.read(16);
        EXPECT_TRUE(maybe_res_real);
        std::vector<uint8_t> res_real = maybe_res_real.value();
        EXPECT_EQ(res, res_real);
    }

    {
        Util::Reader reader({0x13, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00});
        std::vector<uint8_t> res = {0x33, 0x70};
        EXPECT_TRUE(reader.read(4));
        auto maybe_res_real = reader.read(12);
        EXPECT_TRUE(maybe_res_real);
        std::vector<uint8_t> res_real = maybe_res_real.value();
        EXPECT_EQ(res, res_real);
    }

    {
        Util::Reader reader({0x13, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
        std::vector<uint8_t> res = {0x99, 0xb8};
        EXPECT_TRUE(reader.read(3));
        auto maybe_res_real = reader.read(16);
        EXPECT_TRUE(maybe_res_real);
        std::vector<uint8_t> res_real = maybe_res_real.value();
        EXPECT_EQ(res, res_real);
    }

    {
        Util::Reader reader({0x13, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00});
        std::vector<uint8_t> res = {0x6e, 0x80};
        EXPECT_TRUE(reader.read(9));
        auto maybe_res_real = reader.read(13);
        EXPECT_TRUE(maybe_res_real);
        std::vector<uint8_t> res_real = maybe_res_real.value();
        EXPECT_EQ(res, res_real);
    }
}

TEST(testWriter, testArrayWriter)
{
    {
        Util::Writer writer(8);
        std::vector<uint8_t> res = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x00, 0x00}, 16));
        EXPECT_EQ(res, writer.get_payload());
    }

    {
        Util::Writer writer(8);
        std::vector<uint8_t> res = {0x13, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x13, 0x37}, 16));
        EXPECT_EQ(res, writer.get_payload());
    }

    {
        Util::Writer writer(8);
        std::vector<uint8_t> res = {0x03, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x00}, 4));
        EXPECT_TRUE(writer.write({0x33, 0x72}, 12));
        EXPECT_EQ(res, writer.get_payload());
    }

    {
        Util::Writer writer(8);
        std::vector<uint8_t> res = {0x13, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x00}, 3));
        EXPECT_TRUE(writer.write({0x99, 0xb8}, 16));
        EXPECT_EQ(res, writer.get_payload());
    }

    {
        Util::Writer writer(8);
        std::vector<uint8_t> res = {0x00, 0x37, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x00, 0x00}, 9));
        EXPECT_TRUE(writer.write({0x6e, 0x84}, 13));
        EXPECT_EQ(res, writer.get_payload());
    }
}

TEST(testReader, testNumbersReader)
{
    {
        Util::Reader reader({0x13, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x12});
        EXPECT_TRUE(reader.read(9));
        auto res = reader.read_int<uint8_t>(8);
        EXPECT_TRUE(res);
        EXPECT_EQ(static_cast<uint8_t>(0x6e), res.value());
    }
    {
        Util::Reader reader({0x13, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x12});
        EXPECT_TRUE(reader.read(9));
        auto res = reader.read_int<uint16_t>(16);
        EXPECT_TRUE(res);
        EXPECT_EQ(static_cast<uint16_t>(0x6e84), res.value());
    }
    {
        Util::Reader reader({0x13, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x12});
        EXPECT_TRUE(reader.read(9));
        auto res = reader.read_int<uint32_t>(32);
        EXPECT_TRUE(res);
        EXPECT_EQ(static_cast<uint32_t>(0x6e840000), res.value());
    }
    {
        Util::Reader reader({0x13, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x12});
        auto res = reader.read_int<uint64_t>(64);
        EXPECT_TRUE(res);
        EXPECT_EQ(static_cast<uint64_t>(0x1337420000000012), res.value());
    }

    {
        Util::Reader reader({0x00, 0x37, 0x42, 0x12, 0x34, 0x00, 0x00, 0x00});
        EXPECT_TRUE(reader.read(4));
        auto res = reader.read_int<uint8_t>(8);
        EXPECT_TRUE(res);
        EXPECT_EQ(static_cast<uint8_t>(0x03), res.value());
    }
    {
        Util::Reader reader({0x00, 0x37, 0x42, 0x12, 0x34, 0x00, 0x00, 0x00});
        EXPECT_TRUE(reader.read(4));
        auto res = reader.read_int<uint16_t>(16);
        EXPECT_TRUE(res);
        EXPECT_EQ(static_cast<uint16_t>(0x0374), res.value());
    }
    {
        Util::Reader reader({0x00, 0x37, 0x42, 0x12, 0x34, 0x00, 0x00, 0x00});
        EXPECT_TRUE(reader.read(4));
        auto res = reader.read_int<uint32_t>(32);
        EXPECT_TRUE(res);
        EXPECT_EQ(static_cast<uint32_t>(0x03742123), res.value());
    }
    {
        Util::Reader reader({0x00, 0x37, 0x42, 0x12, 0x34, 0x00, 0x00, 0x00});
        EXPECT_TRUE(reader.read(24));
        auto res = reader.read_int<uint64_t>(40);
        EXPECT_TRUE(res);
        EXPECT_EQ(static_cast<uint64_t>(0x0000001234000000), res.value());
    }

    {
        Util::Reader reader({0x13, 0x00, 0x42, 0x53, 0x00, 0x12, 0x34, 0x56});
        EXPECT_TRUE(reader.read(4));
        auto res = reader.read_int<uint8_t>(8);
        EXPECT_TRUE(res);
        EXPECT_EQ(static_cast<uint8_t>(0x30), res.value());
    }
    {
        Util::Reader reader({0x13, 0x00, 0x42, 0x53, 0x00, 0x12, 0x34, 0x56});
        EXPECT_TRUE(reader.read(4));
        auto res = reader.read_int<uint16_t>(16);
        EXPECT_TRUE(res);
        EXPECT_EQ(static_cast<uint16_t>(0x3004), res.value());
    }
    {
        Util::Reader reader({0x13, 0x00, 0x42, 0x53, 0x00, 0x12, 0x34, 0x56});
        EXPECT_TRUE(reader.read(4));
        auto res = reader.read_int<uint32_t>(32);
        EXPECT_TRUE(res);
        EXPECT_EQ(static_cast<uint32_t>(0x30042530), res.value());
    }
    {
        Util::Reader reader({0x13, 0x00, 0x42, 0x53, 0x00, 0x12, 0x34, 0x56});
        EXPECT_TRUE(reader.read(16));
        auto res = reader.read_int<uint64_t>(48);
        EXPECT_TRUE(res);
        EXPECT_EQ(static_cast<uint64_t>(0x0000425300123456), res.value());
    }
}

TEST(testWriter, testNumberWriter)
{
    {
        Util::Writer writer(8);
        std::vector<uint8_t> res;
        res = {0x00, 0x37, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x00, 0x00}, 9));
        EXPECT_TRUE(writer.write_int<uint8_t>(0x6e, 8));
        EXPECT_EQ(res, writer.get_payload());
    }
    {
        Util::Writer writer(8);
        std::vector<uint8_t> res;
        res = {0x00, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x00, 0x00}, 9));
        EXPECT_TRUE(writer.write_int<uint16_t>(0x6e84, 16));
        EXPECT_EQ(res, writer.get_payload());
    }
    {
        Util::Writer writer(8);
        std::vector<uint8_t> res;
        res = {0x00, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x00, 0x00}, 9));
        EXPECT_TRUE(writer.write_int<uint32_t>(0x6e840000, 32));
        EXPECT_EQ(res, writer.get_payload());
    }
    {
        Util::Writer writer(8);
        std::vector<uint8_t> res;
        res = {0x13, 0x37, 0x42, 0x00, 0x00, 0x00, 0x00, 0x12};
        EXPECT_TRUE(writer.write_int<uint64_t>(0x1337420000000012, 64));
        EXPECT_EQ(res, writer.get_payload());
    }

    {
        Util::Writer writer(8);
        std::vector<uint8_t> res;
        res = {0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x00, 0x00}, 4));
        EXPECT_TRUE(writer.write_int<uint8_t>(0x03, 8));
        EXPECT_EQ(res, writer.get_payload());
    }
    {
        Util::Writer writer(8);
        std::vector<uint8_t> res;
        res = {0x00, 0x37, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x00, 0x00}, 4));
        EXPECT_TRUE(writer.write_int<uint16_t>(0x0374, 16));
        EXPECT_EQ(res, writer.get_payload());
    }
    {
        Util::Writer writer(8);
        std::vector<uint8_t> res;
        res = {0x00, 0x37, 0x42, 0x12, 0x30, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x00, 0x00}, 4));
        EXPECT_TRUE(writer.write_int<uint32_t>(0x03742123, 32));
        EXPECT_EQ(res, writer.get_payload());
    }
    {
        Util::Writer writer(8);
        std::vector<uint8_t> res;
        res = {0x00, 0x37, 0x42, 0x12, 0x34, 0x00, 0x00, 0x04};
        EXPECT_TRUE(writer.write({0x00, 0x00}, 8));
        EXPECT_TRUE(writer.write_int<uint16_t>(0x3742, 16));
        EXPECT_TRUE(writer.write_int<uint64_t>(0x0000001234000004, 40));
        EXPECT_EQ(res, writer.get_payload());
    }
    
    {
        Util::Writer writer(8);
        std::vector<uint8_t> res;
        res = {0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x00, 0x00}, 4));
        EXPECT_TRUE(writer.write_int<uint8_t>(0x30, 8));
        EXPECT_EQ(res, writer.get_payload());
    }
    {
        Util::Writer writer(8);
        std::vector<uint8_t> res;
        res = {0x03, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x00, 0x00}, 4));
        EXPECT_TRUE(writer.write_int<uint16_t>(0x3004, 16));
        EXPECT_EQ(res, writer.get_payload());
    }
    {
        Util::Writer writer(8);
        std::vector<uint8_t> res;
        res = {0x03, 0x00, 0x42, 0x53, 0x00, 0x00, 0x00, 0x00};
        EXPECT_TRUE(writer.write({0x00, 0x00}, 4));
        EXPECT_TRUE(writer.write_int<uint32_t>(0x30042530, 32));
        EXPECT_EQ(res, writer.get_payload());
    }
    {
        Util::Writer writer(8);
        std::vector<uint8_t> res;
        res = {0x03, 0x00, 0x42, 0x53, 0x00, 0x12, 0x34, 0x56};
        EXPECT_TRUE(writer.write_int<uint8_t>(0, 4));
        EXPECT_TRUE(writer.write_int<uint8_t>(3, 4));
        EXPECT_TRUE(writer.write({0x00, 0x00}, 8));
        EXPECT_TRUE(writer.write_int<uint64_t>(0x0000425300123456, 48));
        EXPECT_EQ(res, writer.get_payload());
    }
}

TEST(testReader, testFail) {
    {
        Util::Reader reader({0x12, 0x34});
        EXPECT_FALSE(reader.read_int<uint8_t>(10));
        EXPECT_FALSE(reader.read_int<uint32_t>(40));
        EXPECT_FALSE(reader.read(32));
        auto first = reader.read_int<uint8_t>(8);
        EXPECT_TRUE(first);
        EXPECT_EQ(0x12, first.value());
        EXPECT_FALSE(reader.read(9));
        auto second = reader.read(8);
        EXPECT_TRUE(second);
        EXPECT_EQ(std::vector<uint8_t>{0x34}, second.value());
        EXPECT_FALSE(reader.read(1));
    }
}

TEST(testWriter, testFail) {
    {
        Util::Writer writer(2); 
        EXPECT_FALSE(writer.write_int<uint8_t>(0x32, 9));
        EXPECT_FALSE(writer.write_int<uint32_t>(0x334, 40));
        EXPECT_FALSE(writer.write({0x00}, 10));
        EXPECT_FALSE(writer.write({0x01, 0x02, 0x03}, 24));
        EXPECT_TRUE(writer.write_int<uint8_t>(0x32, 8));
        EXPECT_FALSE(writer.write_int<uint16_t>(4, 9));
        EXPECT_TRUE(writer.write({0x54}, 8));
        EXPECT_EQ((std::vector<uint8_t>{0x32, 0x54}), writer.get_payload());
    }
}

TEST(testWriter, testDynamicWriter) {
    Util::DynamicWriter writer{};
    EXPECT_TRUE(writer.write_int<uint8_t>(0x42, 8));
    EXPECT_TRUE(writer.write_int<uint16_t>(0x1234, 12));
    EXPECT_TRUE(writer.write({0x67, 0x98}, 16));
    EXPECT_EQ((std::vector<uint8_t>{0x42, 0x23, 0x46, 0x79, 0x80}), writer.get_payload());
}
