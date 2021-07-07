#include "datatypes.h"

std::map<uint16_t, int> Can::Data::m_sizes = {
    {static_cast<uint16_t>(DataIdentifier::VIN), 17 * 8},
    {static_cast<uint16_t>(DataIdentifier::UPASystemType), 8},
    {static_cast<uint16_t>(DataIdentifier::THRFOUPA), 120},
    {static_cast<uint16_t>(DataIdentifier::Conf), 16 + 24 + 16}
};

std::map<uint16_t, std::string> Can::Data::m_names = {
    {static_cast<uint16_t>(DataIdentifier::VIN), "VIN"},
    {static_cast<uint16_t>(DataIdentifier::UPASystemType), "UPASystemType"},
    {static_cast<uint16_t>(DataIdentifier::THRFOUPA), "THRFOUPA"},
    {static_cast<uint16_t>(DataIdentifier::Conf), "Conf"}
};
