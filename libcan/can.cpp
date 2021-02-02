#include "can.h"

#include <array>
#include <cstdint>

Can::Frame::Frame(std::array<uint8_t, 8> payload) {
    FrameInfo info;
    info.value = payload[0];
    this->type = FrameType(info.data.type);
    this->size = info.data.size;
}
