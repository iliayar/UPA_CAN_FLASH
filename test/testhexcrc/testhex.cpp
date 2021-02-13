#include <string>
#include <vector>

#include "crc.h"
#include "gtest/gtest.h"
#include "hex.h"

TEST(tesHexCRC, testCRC) {
    std::vector<uint8_t> c = {1, 2, 3, 4};
    uint16_t v = Util::crc16_block(c, 0);
    EXPECT_EQ(v, 3331);
}

TEST(testHexCRC, testHexCRC) {
    std::string hex =
        ":020000040800F2\n\
:045170000000000437\n\
:0400000508002CEDD6\n\
:00000001FF\n";
    Hex::HexReader reader(new Hex::StringSource(hex));
    Hex::HexInfo info = Hex::read_hex_info(reader);
    EXPECT_EQ(info.size, 4);
    EXPECT_EQ(info.crc, 0xC444);
    EXPECT_EQ(info.start_addr, 0x08005170);
}

TEST(testHecCTC, testReadingData) {
    int max_block_size = 1024;
    std::vector<uint8_t> data(max_block_size, 0);
    int i = 0;
    int block_counter = 1;
    int n_size = 0;
    std::string hex =
        ":020000040800F2\n\
:045170000000000437\n\
:0400000508002CEDD6\n\
:00000001FF\n";
    Hex::HexReader reader(new Hex::StringSource(hex));
    while (!reader.is_eof()) {
        Hex::HexLine *line = reader.read_line();
        if (line->get_type() == Hex::HexLineType::Data ||
            line->get_type() == Hex::HexLineType::EndOfFile) {
            std::vector<uint8_t> line_data;
            if (line->get_type() == Hex::HexLineType::Data) {
                line_data = static_cast<Hex::DataLine *>(line)->get_data();
            } else {
                line_data = {data[i-1]};
                data.resize(i);
                i--;
            }
            for (uint8_t d : line_data) {
                data[i++] = d;
                n_size++;
                if (i >= data.size()) {
                    EXPECT_EQ(data, std::vector<uint8_t>({0x00, 0x00, 0x00, 0x00}));
                    i = 0;
                }
            }
        }
    }
}
