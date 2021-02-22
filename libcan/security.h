/**
 * @file security.h
 * Provide functions to pass security access
 * to enter appropriate mode in ECU
 */
#pragma once

#include <cstdint>
#include <cstdlib>
#include <ctime>

#include "config.h"

/**
 * Confidencial masks to pass security check
 * to flash ECU
 */
#ifndef MASK03
#define MASK03 0x00000000
#endif

#ifndef MASK02
#define MASK02 0x00000000
#endif

namespace Crypto {

/**
 * @return random 1 byte parameter for received seed
 */
uint8_t get_RND() {
    std::srand(std::time(nullptr));

    return std::rand();
}

/**
 * Calcualte received seed to key
 * @param received seed
 * @param generated earlier random seed parameter
 * @param mask for particular mode to enter in
 * @return 4 bytes key to send to ECU
 */
static uint32_t seed_to_key(uint32_t seed, uint8_t rnd, uint32_t mask) {
    return key ^ seed;
}

/**
 * Uses MASK03 in seed_to_key function to
 * enter programming mode
 */
uint32_t seed_to_key_03(uint32_t seed, uint8_t rnd) {
    return seed_to_key(seed, rnd, MASK03);
}

/**
 * Uses MASK02 in seed_to_key function to
 * enter diagnostic? mode
 */
uint32_t seed_to_key_02(uint32_t seed, uint8_t rnd) {
    return seed_to_key(seed, rnd, MASK02);
}

}  // namespace Crypto

#undef MASK03
#undef MASK02
#undef RND
