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
    std::shared_ptr<Hex::HexLine> line;

    line = reader.read_line();
    EXPECT_EQ(line->get_type(), Hex::HexLineType::ExtendLinearAddress);
    EXPECT_EQ(
        static_cast<Hex::ExtendLinearAddressLine*>(line.get())->get_address(),
        0x0800);

    line = reader.read_line();
    EXPECT_EQ(line->get_type(), Hex::HexLineType::Data);
    EXPECT_EQ(static_cast<Hex::DataLine*>(line.get())->get_address(), 0x0000);
    EXPECT_EQ(
        static_cast<Hex::DataLine*>(line.get())->get_data(),
        std::vector<uint8_t>({0x50, 0x22, 0x00, 0x20, 0x69, 0x47, 0x01, 0x08,
                              0x71, 0x47, 0x01, 0x08, 0x73, 0x47, 0x01, 0x08}));

    line = reader.read_line();
    EXPECT_EQ(line->get_type(), Hex::HexLineType::Data);
    EXPECT_EQ(static_cast<Hex::DataLine*>(line.get())->get_address(), 0xfff0);
    EXPECT_EQ(
        static_cast<Hex::DataLine*>(line.get())->get_data(),
        std::vector<uint8_t>({0x21, 0xD1, 0x9B, 0x4B, 0x80, 0x33, 0x1A, 0x7F,
                              0x90, 0x06, 0x1C, 0xD4, 0xD9, 0x7F, 0x88, 0x06}));

    line = reader.read_line();
    EXPECT_EQ(line->get_type(), Hex::HexLineType::ExtendLinearAddress);
    EXPECT_EQ(
        static_cast<Hex::ExtendLinearAddressLine*>(line.get())->get_address(),
        0x0801);

    line = reader.read_line();
    EXPECT_EQ(line->get_type(), Hex::HexLineType::StartLinearAddress);
    EXPECT_EQ(
        static_cast<Hex::StartLinearAddressLine*>(line.get())->get_address(),
        0x080000ed);

    line = reader.read_line();
    EXPECT_EQ(line->get_type(), Hex::HexLineType::EndOfFile);

    EXPECT_EQ(reader.is_eof(), true);
}
