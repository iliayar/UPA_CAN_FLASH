#include "config.h"

DataConfig::DataConfig() {
    fields = {{"Data",
               {
                   new StringField("VIN", Can::DataIdentifier::VIN, 17 * 8),
                   new EnumField(
                       "ECU Type", Can::DataIdentifier::UPASystemType,
                       {
                           {"not configured", 0x00},
                           {"Vesta non-cross-12", 0x08},
                           {"Vesta cross-12", 0x09},
                       },
                       8),
               }}};
}
