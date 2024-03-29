#include "gtest/gtest.h"
#include "security.h"

TEST(testCrypto, testCrypto) {
    {
	    uint8_t rnd = 0x33;
	    uint32_t seed = 0x11223344;
	    EXPECT_EQ(Crypto::seed_to_key_02(seed, rnd), seed ^ Crypto::SecuritySettings::get_mask02());
    }

    {
	    uint8_t rnd = 0x33;
	    uint32_t seed = 0x11223344;
	    EXPECT_EQ(Crypto::seed_to_key_03(seed, rnd), seed ^ Crypto::SecuritySettings::get_mask03());
    }
}
