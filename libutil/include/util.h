/**
 * @file util.h
 * Some usefull funcions
 */
#pragma once

#include <string>
#include <vector>
#include <experimental/filesystem>

namespace Util {

/**
 * Convert vector of bytes to string
 * @param vector of bytes
 * @return appropriate string
 */
std::string vec_to_str(std::vector<uint8_t> const& vec);

/**
 * Convert string to vector of bytes
 * @param string
 * @return appropriate vector of bytes
 */
std::vector<uint8_t> str_to_vec(std::string const& str);

/**
 * Converts int value to hex string
 * @param n integer valuea
 * @return string hex representation of {@code value}
 */
std::string int_to_hex(int n);
}  // namespace Util

#ifdef __MINGW32__
#define FILEPATH(file) std::experimental::filesystem::u8path(file).wstring().c_str()
#elif __linux__
#define FILEPATH(file) file
#endif
