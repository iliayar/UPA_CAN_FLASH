#include "util.h"

std::string Util::vec_to_str(std::vector<uint8_t> vec) {
    std::string res;
    for (uint8_t c : vec) {
        res += static_cast<char>(c);
    }
    return res;
}

std::vector<uint8_t> Util::str_to_vec(std::string str) {
    std::vector<uint8_t> res;
    for (char c : str) {
        res.push_back(static_cast<uint8_t>(c));
    }
    return res;
}
