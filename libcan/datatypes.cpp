#include "datatypes.h"

using namespace Can;

std::map<uint16_t, int> Data::m_sizes = {
    {static_cast<uint16_t>(DataIdentifier::VIN), 17 * 8},
    {static_cast<uint16_t>(DataIdentifier::UPASystemType), 8},
    {static_cast<uint16_t>(DataIdentifier::THRFOUPA), 120},
    {static_cast<uint16_t>(DataIdentifier::Conf), 16 + 24 + 16}
};
