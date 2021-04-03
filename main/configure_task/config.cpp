#include "config.h"

DataConfig::DataConfig() {
    fields = {{"Data",
               {
                   new StringField("VIN", Can::DataIdentifier::VIN, 17 * 8),
                   new EnumField(
                       "ECU Type", Can::DataIdentifier::UPASystemType,
                       {
                           {"not configured", 0x00},
                           {"Vesta non-cross-4", 0x01},
                           {"Vesta non-cross -8", 0x02},
                           {"Vesta cross-4", 0x03},
                           {"Vesta cross -8", 0x04},
                           {"Largus-7", 0x06},
                           {"Largus-3", 0x07},
                       },
                       8),
               }}};
}
