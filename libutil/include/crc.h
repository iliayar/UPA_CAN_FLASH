/**
 * @file crc.h
 * implemets CRC sum function
 */
#pragma once

#include <cstdint>
#include <vector>

namespace Util {

/**
 * Calculated crc of 4 bytes block
 * @param 4 byte block
 * @param previous calculated CRC. A new one is depends on it
 */
uint16_t crc16_block(std::vector<uint8_t> const& block, uint16_t val);

}  // namespace Util
