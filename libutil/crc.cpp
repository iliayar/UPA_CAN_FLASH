#include "crc.h"

uint16_t Util::crc16_block(std::vector<uint8_t> block, uint16_t val) {

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
