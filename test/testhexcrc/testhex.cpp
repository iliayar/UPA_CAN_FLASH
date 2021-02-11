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
    std::ifstream fin("./test.hex");
    EXPECT_EQ(!fin, false);
    Hex::HexReader reader{new Hex::FileSource(fin)};
    int size = 0;
    uint16_t crc = 0xffff; 
    std::vector<uint8_t> last_4(4, 0);
    int last_4_i = 0;
    while (!reader.is_eof()) {
        Hex::HexLine *line = reader.read_line();
        if (line->get_type() == Hex::HexLineType::Data) {
            std::vector<uint8_t> line_data = static_cast<Hex::DataLine*>(line)->get_data();
            for (uint8_t d : line_data) {
                last_4[last_4_i++] = d;
                size++;
                if (last_4_i >= 4) {
                    crc = Util::crc16_block(last_4, crc);
                    // std::cout << "CRC: " << std::hex << crc << std::endl;
                    last_4_i = 0;
                }
            }
            if (line->get_type() == Hex::HexLineType::EndOfFile) {
                EXPECT_EQ(reader.is_eof(), true);
                break;
            }
        }
    }
    fin.close();
    if(last_4_i != 0) {
        while(last_4_i < 4) last_4[last_4_i++] = 0;
        crc = Util::crc16_block(last_4, crc);
    }
    EXPECT_EQ(size, 0x12578);
    EXPECT_EQ(0xe78e, crc);
}
