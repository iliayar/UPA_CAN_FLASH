#include "bytes.h"

#include <cstdint>
#include <functional>
#include <stdexcept>

Util::Reader::Reader(std::vector<uint8_t> payload)
    : m_payload(payload), m_offset(0) {}

optional<std::vector<uint8_t>> Util::Reader::read(int len) {
    if (len + m_offset > m_payload.size() * 8) {
        return {};
    }
    std::vector<uint8_t> res(BYTES(len), 0);
    int bits_read = 0;
    while (bits_read < len) {
        int byte_offset = (m_offset + bits_read) % 8;
        int byte_num = (m_offset + bits_read) / 8;
        uint8_t mask = 0xff >> byte_offset;
        int n = m_payload[byte_num] & mask;
        int to_read = std::min(8 - byte_offset,
                               std::min(8 - (bits_read % 8), len - bits_read));
        mask = 0xff << (8 - to_read);
        n = ((n << (byte_offset)) & mask) >> (bits_read % 8);
        res[bits_read / 8] |= n;

        bits_read += to_read;
    }
    m_offset += len;
    return res;
}

Util::Writer::Writer(int size)
    : m_payload(size, 0), m_offset(0) {}

bool Util::Writer::write(std::vector<uint8_t> data, int len) {
    while(data.size()*8 < len) {
        data.push_back(0);
    }
    if (len + m_offset > m_payload.size() * 8) {
        return false;
    }
    int bits_wrote = 0;
    while (bits_wrote < len) {
        int byte_offset = (m_offset + bits_wrote) % 8;
        int byte_num = (m_offset + bits_wrote) / 8;
        uint8_t mask = 0xff >> (bits_wrote % 8);
        int n = data[bits_wrote / 8] & mask;
        int to_write = std::min(
            8 - byte_offset, std::min(8 - (bits_wrote % 8), len - bits_wrote));
        mask = 0xff << (8 - to_write);
        n = ((n << (bits_wrote % 8)) & mask) >> (byte_offset);
        m_payload[byte_num] |= n;

        bits_wrote += to_write;
    }
    m_offset += len;
    return true;
}
