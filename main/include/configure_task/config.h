#pragma once

// #include "fields.h"
#include <unordered_map>
#include <vector>
#include <memory>
#include <string>
#include <QString>
class Field;

struct DataConfig {
public:

    DataConfig();

    std::vector<std::pair<std::string, std::vector<Field*>>> fields;
    std::unordered_map<
        uint16_t, std::pair<std::string,
                            std::unordered_map<
                                uint8_t, std::pair<std::string, std::string>>>>
        errors;

    void group_to_json(QString filename, std::string group);
    void json_to_group(QString filename, std::string group);

};


