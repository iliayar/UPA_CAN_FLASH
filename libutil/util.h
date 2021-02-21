/**
 * @file util.h
 * Some usefull funcions
 */
#pragma once

#include <string>
#include <vector>

namespace Util {

/**
 * Convert vector of bytes to string
 * @param vector of bytes
 * @return appropriate string
 */
std::string vec_to_str(std::vector<uint8_t> vec);

/**
 * Convert string to vector of bytes
 * @param string
 * @return appropriate vector of bytes
 */
std::vector<uint8_t> str_to_vec(std::string str);
}  // namespace Util
