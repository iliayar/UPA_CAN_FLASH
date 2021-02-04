#pragma once

#include <cstdint>
#include <vector>

#define BYTES(n) (((n) + 7)/8)

namespace Can {


    class Reader {
    public:
        Reader(std::vector<uint8_t>);
        
        uint8_t              read_8 (int offset, int len);
        uint16_t             read_16(int offset, int len);
        uint32_t             read_32(int offset, int len);
        uint64_t             read_64(int offset, int len);
        std::vector<uint8_t> read   (int offset, int len);

    private:
        std::vector<uint8_t> m_payload;
    };

    class Writer {
    public:
        Writer(std::vector<uint8_t>&);
        
        void write_8 (uint8_t              data, int offset, int len);
        void write_16(uint16_t             data, int offset, int len);
        void write_32(uint32_t             data, int offset, int len);
        void write_64(uint64_t             data, int offset, int len);
        void write   (std::vector<uint8_t> data, int offset, int len);

    private:
        std::vector<uint8_t>& m_payload;
    };
}
