#pragma once

#include "fields.h"

struct DataConfig {
public:

    DataConfig();

    std::vector<std::pair<std::string, std::vector<Field*>>> fields;

};


