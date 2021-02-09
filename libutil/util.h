#pragma once

#include <string>
#include <vector>

namespace Util {
std::string vec_to_str(std::vector<uint8_t> vec);
std::vector<uint8_t> str_to_vec(std::string str);
}  // namespace Util
