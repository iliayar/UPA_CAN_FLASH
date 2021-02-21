/**
 * @file bytes.h
 * Utils for reading/writing raw bits
 * from/to unsigned integers, vectors of uint8_t
 */
#pragma once

#include <cstdint>
#include <vector>

#define BYTES(n) (((n) + 7) / 8)

namespace Util {

/**
 * Class to read from vector of bytes
 * Can read arbitrary count of BITS.
 * If count is not divided by 8
 * puts it in the leasts bits of readed value
 * other bits are zero
 */
class Reader {
public:
    /**
     * @param vector of raw bytes to read from
     */
    Reader(std::vector<uint8_t>);

    /**
     * @param offset in intial vector to read from
     * @param counts of BITS to read from initial vector
     * @return 8 bit value
     */
    uint8_t read_8(int offset, int len);

    /**
     * @param offset in intial vector to read from
     * @param counts of BITS to read from initial vector
     * @return 16 bit value
     */
    uint16_t read_16(int offset, int len);

    /**
     * @param offset in intial vector to read from
     * @param counts of BITS to read from initial vector
     * @return 32 bit value
     */
    uint32_t read_32(int offset, int len);

    /**
     * @param offset in intial vector to read from
     * @param counts of BITS to read from initial vector
     * @return 64 bit value
     */
    uint64_t read_64(int offset, int len);

    /**
     * @param offset in intial vector to read from
     * @param counts of BITS to read from initial vector
     * @return vector of readed bits. The length of vector is
     * (len + 7) / 8
     */
    std::vector<uint8_t> read(int offset, int len);

    /**
     * Increase inner offset of reader
     * @param count of bits to increas offset to
     */
    void add_offset(int offset);

    /**
     * @return inner offset
     */
    int get_offset() { return m_offset; }

    /**
     * @return true if end of data is reached
     */
    bool is_eof() { return m_offset >= m_payload.size(); }

private:
    std::vector<uint8_t> m_payload;
    int m_offset;
};

/**
 * Class to write to vector of bytes
 * Can write arbitrary count of BITS.
 * If count is not divided by 8
 * write the leasts bits of passed value
 * other bits are counted as zeros
 */
class Writer {
public:
    /**
     * @param reference to vector of raw bytes to wtite to
     */
    Writer(std::vector<uint8_t>&);

    /**
     * @param 16 BITS value to write to initial vector
     * @param offset in intial vector to write to
     * @param counts of BITS to wite to initial vector
     */
    void write_8(uint8_t data, int offset, int len);

    /**
     * @param 16 BITS value to write to initial vector
     * @param offset in intial vector to write to
     * @param counts of BITS to wite to initial vector
     */
    void write_16(uint16_t data, int offset, int len);

    /**
     * @param 32 BITS value to write to initial vector
     * @param offset in intial vector to write to
     * @param counts of BITS to wite to initial vector
     */
    void write_32(uint32_t data, int offset, int len);

    /**
     * @param 64 BITS value to write to initial vector
     * @param offset in intial vector to write to
     * @param counts of BITS to wite to initial vector
     */
    void write_64(uint64_t data, int offset, int len);

    /**
     * @param data to write to initial vector
     * @param offset in intial vector to write to
     * @param counts of BITS to wite to initial vector
     * (len + 7) / 8
     */
    void write(std::vector<uint8_t> data, int offset, int len);

    /**
     * Increase inner offset of writer
     * @param count of bits to increas offset to
     */
    void add_offset(int offset);

    /**
     * @return inner offset
     */
    int get_offset() { return m_offset; }

    /**
     * @return true if end of data is reached
     */
    bool is_eof() { return m_offset >= m_payload.size(); }

private:
    std::vector<uint8_t>& m_payload;
    int m_offset;
};
}  // namespace Util
