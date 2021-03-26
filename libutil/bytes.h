/**
 * @file bytes.h
 * Utils for reading/writing raw bits
 * from/to unsigned integers, vectors of uint8_t
 */
#pragma once

#include <cstdint>
#include <memory>
#include <vector>
#include <functional>
#include "templates.h"

#define BYTES(n) (((n) + 7) / 8)

#include <experimental/optional>
using std::experimental::optional;

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
     * @param data vector of raw bytes to read from
     */
    Reader(std::vector<uint8_t> data);

    /**
     * @param len counts of BITS to read from initial vector
     * @return optional of vector of readed bits. The length of vector is
     * (len + 7) / 8
     */
    optional<std::vector<uint8_t>> read(int len);

    /**
     * @return true if end of data is reached
     */
    bool is_eof() { return m_offset >= m_payload.size()*8; }

    /**
     * Reads to lowest bits of resulting number
     * @tparam T type to read to, checking it's size
     * @param len counts of BITS to read from initial vector
     * @return optional 64 bit value
     */
    template <typename T>
    optional<T> read_int(int len) {
        if (len > sizeof(T)*8) {
            return {};
        }
        auto vec = this->read(len);
        if (!vec) {
            return {};
        }
        std::vector<uint8_t> res_vec = std::move(vec.value());
        while (res_vec.size() < sizeof(T)) {
            res_vec.push_back(0);
        }
        T res;
        uint8_t* ptr = (uint8_t*)(&res);
        for (int i = 0; i < sizeof(T); ++i) {
            *ptr &= 0x00;
            ptr++;
        }
        for (int i = 0; i < sizeof(T); ++i) {
            res <<= 8;
            res |= res_vec[i];
        }
        return res >> (sizeof(T)*8 - len);
    }

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
     * @param size of resulting data
     */
    Writer(int size);

    /**
     * @param data data to write to initial vector
     * @param len counts of BITS to wite to initial vector
     * @return false if unable to write data, true otherwise
     * (len + 7) / 8
     */
    virtual bool write(std::vector<uint8_t> data, int len);

    /**
     * @return true if end of data is reached
     */
    bool is_eof() { return m_offset >= m_payload.size(); }

    /**
     * @param size new size of resulting payload
     */
    void resize(int size) { m_payload.resize(size); }

    std::vector<uint8_t> get_payload() { return m_payload; }

    /**
     * Write from lowes bits of provided number
     * @tparam T type of value to write in, checking its size
     * @param data 16 BITS value to write to initial vector
     * @param len counts of BITS to wite to initial vector
     * @return false if unable to write value, true otherwise
     */
    template <typename T>
    bool write_int(T data, int len) {
        if (len > sizeof(T)*8) {
            return false;
        }
        data <<= (sizeof(T)*8 - len);
        std::vector<uint8_t> data_vec(sizeof(T), 0);
        for (int i = sizeof(T) - 1; i >= 0; --i) {
            data_vec[i] |= data;
            data >>= 8;
        }
        return this->write(data_vec, len);
    }

protected:
    std::vector<uint8_t> m_payload;
    int m_offset;
};

/**
 * Class similar to {@link Writer}, but automatically allocates array for data
 */
class DynamicWriter : public Writer {
public:
    DynamicWriter() : Writer(0) {}

    /**
     * @param data data to write
     * @param size size of data
     * @return true always, because of automatically alocations
     */
    bool write(std::vector<uint8_t> data, int size) override {
        if(m_offset + size > m_payload.size()*8) {
            m_payload.resize(BYTES(m_offset + size));
        }
        return Writer::write(data, size);
    }
};

}  // namespace Util
