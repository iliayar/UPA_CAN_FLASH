#include "gtest/gtest.h"
#include "hex.h"
#include "crc.h"

#include <string>
#include <vector>

TEST(tesHexCRC, testCRC)
{
    std::vector<uint8_t> c = {1, 2, 3, 4};
    uint16_t v = Util::crc16_block(c, 0);
    EXPECT_EQ(v, 3331);
}


TEST(testHexCRC, testHexCRC)
{
    std::string hex = ":020000040800F2\n\
:045170000000000437\n\
:0400000508002CEDD6\n\
:00000001FF\n";
    Hex::HexReader reader(new Hex::StringSource(hex));
    Hex::HexInfo info = Hex::read_hex_info(reader);
    EXPECT_EQ(info.size, 4);
    EXPECT_EQ(info.crc, 0xC444);
    EXPECT_EQ(info.start_addr, 0x08005170);
}
