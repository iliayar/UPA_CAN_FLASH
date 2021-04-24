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
 * Global Settings for security access
 */
class SecuritySettings {
public:

    /**
     * @param mask02 value to be global now
     */
    static void set_mask02(uint32_t v);

    /**
     * @param mask03 value to be global now
     */
    static void set_mask03(uint32_t v);

    /**
     * @return mask02 global value
     */
    static uint32_t get_mask02();

    /**
     * @return mask03 global value
     */
    static uint32_t get_mask03();

private:
    static uint32_t m_mask02;
    static uint32_t m_mask03;
};

/**
 * @return random 1 byte parameter for received seed
 */
uint8_t get_RND();

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
 * enter diagnostic mode
 */
    uint32_t seed_to_key_03(uint32_t seed, uint8_t rnd);

/**
 * Uses MASK02 in seed_to_key function to
 * enter programming mode
 */
uint32_t seed_to_key_02(uint32_t seed, uint8_t rnd);
}  // namespace Crypto
