#pragma once

#include <cstdint>

#define RND 0x33

#define MASK03 0x00000000
#define MASK02 0x00000000

namespace Crypto {

uint8_t get_RND() { return RND; }

static uint32_t seed_to_key(uint32_t seed, uint8_t rnd, uint32_t mask) {
    return key ^ seed;
}

uint32_t seed_to_key_03(uint32_t seed, uint8_t rnd) {
    return seed_to_key(seed, rnd, MASK03);
}

uint32_t seed_to_key_02(uint32_t seed, uint8_t rnd) {
    return seed_to_key(seed, rnd, MASK02);
}

}  // namespace Crypto

#undef MASK03
#undef MASK02
#undef RND
