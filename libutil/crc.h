#pragma once

#include <cstdint>
#include <vector>

namespace Util {

// uint16_t crc16_block(std::vector<uint8_t> block, uint16_t val) {
//     uint16_t fwCRC = val;
//     for(uint8_t b : block) {
//         fwCRC ^= b << 8;
//         for(int i = 0; i < 8; ++i) {
//             fwCRC = fwCRC & 0x8000 ? (fwCRC << 1) ^ 0x1021 : fwCRC << 1;
//         }
//     }
//     return fwCRC;
// }
uint16_t crc16_block(std::vector<uint8_t> block, uint16_t val) {

    uint16_t len = block.size();
    uint8_t *pcBlock = block.data();

    unsigned char i;
    unsigned int fwCRC = val;

    while (len--) {
        fwCRC ^= *pcBlock++ << 8;
        for (i = 0; i < 8; i++)
            fwCRC = fwCRC & 0x8000 ? (fwCRC << 1) ^ 0x1021 : fwCRC << 1;
    }

    return fwCRC;
}

}  // namespace Util