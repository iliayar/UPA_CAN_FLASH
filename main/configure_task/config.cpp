#include "config.h"

DataConfig::DataConfig() {
    fields = {{"Data",
               {
                   new StringField("VIN", Can::DataIdentifier::VIN, 17 * 8),
                   new EnumField("ECU Type", Can::DataIdentifier::UPASystemType,
                                 {
                                     {"not configured", 0x00},
                                     {"Vesta non-cross-12", 0x08},
                                     {"Vesta cross-12", 0x09},
                                 },
                                 8),
                   new MultiField("Conf", Can::DataIdentifier::Conf,
                                  {
                                      {16, "Version"},
                                      {24, "Date"},
                                      {16, "Checksum"},
                                  }),
                   new VecField("Front Outer Sensors Thresholds UPA",
                                Can::DataIdentifier::THRFOUPA, 120),
               }}};
}
