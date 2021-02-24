#include "security.h"

#include "config.h"

uint32_t Crypto::SecuritySettings::m_mask02 = MASK02;
uint32_t Crypto::SecuritySettings::m_mask03 = MASK03;

void Crypto::SecuritySettings::set_mask02(uint32_t v) { m_mask02 = v; }

void Crypto::SecuritySettings::set_mask03(uint32_t v) { m_mask03 = v; }

uint32_t Crypto::SecuritySettings::get_mask02() { return m_mask02; }

uint32_t Crypto::SecuritySettings::get_mask03() { return m_mask03; }

uint8_t Crypto::get_RND() {
    std::srand(std::time(nullptr));

    return std::rand();
}

uint32_t Crypto::seed_to_key_03(uint32_t seed, uint8_t rnd) {
    return seed_to_key(seed, rnd, SecuritySettings::get_mask03());
}
uint32_t Crypto::seed_to_key_02(uint32_t seed, uint8_t rnd) {
    return seed_to_key(seed, rnd, SecuritySettings::get_mask02());
}
