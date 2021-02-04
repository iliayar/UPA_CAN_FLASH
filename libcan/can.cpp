#include "can.h"

#include <cstdint>
#include <stdexcept>

#include <iostream>

Can::Reader::Reader(std::vector<uint8_t> payload)
    : m_payload(payload)
{}

std::vector<uint8_t> Can::Reader::read(int offset, int len)
{
    if(len + offset > m_payload.size()*8) {
        throw std::runtime_error("Len with offset is greater than frame size");
    }
    std::vector<uint8_t> res(BYTES(len), 0);
    int bits_read = 0;
    while(bits_read < len) {
        int byte_offset = (offset + bits_read) % 8;
        int byte_num    = (offset + bits_read) / 8;
        uint8_t mask    = 0xff >> byte_offset;
        int n           = m_payload[byte_num] & mask;
        int to_read     = std::min(8 - byte_offset, std::min(8 - (bits_read % 8), len - bits_read));
        mask            = 0xff << (8 - byte_offset - to_read);
        n               = ((n << (byte_offset)) & mask) >> (bits_read % 8);
        res[bits_read / 8] |= n;

        bits_read += to_read;
    }
    return res;
}

#define READ_N(N)                                                       \
    uint##N##_t Can::Reader::read_##N(int offset, int len) {       \
        if(len > N) {                                                   \
            throw std::runtime_error("Len is greater than size of uint"#N"_t"); \
        }                                                               \
        std::vector<uint8_t> res_vec = this->read(offset, len);         \
        while(res_vec.size() < N / 8) {                                 \
            res_vec.push_back(0);                                       \
        }                                                               \
        uint##N##_t res;                                                \
        for(int i = 0; i < N / 8; ++i) {                                \
            res <<= 8;                                                  \
            res |= res_vec[i];                                          \
        }                                                               \
        return res >> (N - len);                                        \
    }
READ_N(8)
READ_N(16)
READ_N(32)
READ_N(64)
#undef READ_N

Can::Writer::Writer(std::vector<uint8_t>& payload)
    : m_payload(payload)
{}

void Can::Writer::write(std::vector<uint8_t> data, int offset, int len)
{
    if(len + offset > m_payload.size()*8) {
        throw std::runtime_error("Len with offset is greater than frame size");
    }
    int bits_wrote = 0;
    while(bits_wrote < len) {
        int byte_offset = (offset + bits_wrote) % 8;
        int byte_num    = (offset + bits_wrote) / 8;
        uint8_t mask    = 0xff >> (bits_wrote % 8);
        int n           = data[bits_wrote / 8] & mask;
        int to_write    = std::min(8 - byte_offset, std::min(8 - (bits_wrote % 8), len - bits_wrote));
        mask            = 0xff << (8 - byte_offset - to_write);
        n               = ((n << (bits_wrote % 8)) & mask) >> (byte_offset);
        m_payload[byte_num] |= n;

        bits_wrote += to_write;
    }
}
