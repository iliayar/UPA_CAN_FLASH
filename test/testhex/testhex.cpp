#include <string>
#include <vector>

#include "gtest/gtest.h"
#include "hex.h"

TEST(testHex, testStrToBytes) {
    {
        std::string str = "01020342";
        std::vector<uint8_t> res = Hex::str_to_bytes(str);
        EXPECT_EQ(res, std::vector<uint8_t>({0x01, 0x02, 0x03, 0x42}));
    }

    {
        std::string str = "DEADbeef";
        std::vector<uint8_t> res = Hex::str_to_bytes(str);
        EXPECT_EQ(res, std::vector<uint8_t>({0xde, 0xad, 0xbe, 0xef}));
    }
}

TEST(testHex, testStringReader) {
    std::shared_ptr<Hex::StringSource> src =
        std::make_shared<Hex::StringSource>(
            ":020000040800F2\n\
:100000005022002069470108714701087347010821\n\
:10FFF00021D19B4B80331A7F90061CD4D97F880671\n\
:020000040801F1\n\
:04000005080000ED02\n\
:00000001FF\n\
");
    Hex::HexReader reader(src);
    std::shared_ptr<Hex::Line::Line> line;

    {
        auto maybe_line = reader.read_line();
        EXPECT_TRUE(maybe_line);
        line = maybe_line.value();
    }
    EXPECT_EQ(line->get_type(), Hex::Line::Type::ExtendLinearAddress);
    EXPECT_EQ(
        std::static_pointer_cast<Hex::Line::ExtendLinearAddress>(line)->get_address(),
        0x0800);

    {
        auto maybe_line = reader.read_line();
        EXPECT_TRUE(maybe_line);
        line = maybe_line.value();
    }
    EXPECT_EQ(line->get_type(), Hex::Line::Type::Data);
    EXPECT_EQ(std::static_pointer_cast<Hex::Line::Data>(line)->get_address(), 0x0000);
    EXPECT_EQ(
        std::static_pointer_cast<Hex::Line::Data>(line)->get_data(),
        std::vector<uint8_t>({0x50, 0x22, 0x00, 0x20, 0x69, 0x47, 0x01, 0x08,
                              0x71, 0x47, 0x01, 0x08, 0x73, 0x47, 0x01, 0x08}));

    {
        auto maybe_line = reader.read_line();
        EXPECT_TRUE(maybe_line);
        line = maybe_line.value();
    }
    EXPECT_EQ(line->get_type(), Hex::Line::Type::Data);
    EXPECT_EQ(std::static_pointer_cast<Hex::Line::Data>(line)->get_address(), 0xfff0);
    EXPECT_EQ(
        std::static_pointer_cast<Hex::Line::Data>(line)->get_data(),
        std::vector<uint8_t>({0x21, 0xD1, 0x9B, 0x4B, 0x80, 0x33, 0x1A, 0x7F,
                              0x90, 0x06, 0x1C, 0xD4, 0xD9, 0x7F, 0x88, 0x06}));

    {
        auto maybe_line = reader.read_line();
        EXPECT_TRUE(maybe_line);
        line = maybe_line.value();
    }
    EXPECT_EQ(line->get_type(), Hex::Line::Type::ExtendLinearAddress);
    EXPECT_EQ(
        std::static_pointer_cast<Hex::Line::ExtendLinearAddress>(line)->get_address(),
        0x0801);

    {
        auto maybe_line = reader.read_line();
        EXPECT_TRUE(maybe_line);
        line = maybe_line.value();
    }
    EXPECT_EQ(line->get_type(), Hex::Line::Type::StartLinearAddress);
    EXPECT_EQ(
        std::static_pointer_cast<Hex::Line::StartLinearAddress>(line)->get_address(),
        0x080000ed);

    {
        auto maybe_line = reader.read_line();
        EXPECT_TRUE(maybe_line);
        line = maybe_line.value();
    }
    EXPECT_EQ(line->get_type(), Hex::Line::Type::EndOfFile);

    EXPECT_EQ(reader.is_eof(), true);
}
