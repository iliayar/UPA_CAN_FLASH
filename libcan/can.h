#pragma once

#include <cstdint>
#include <array>

namespace Can {

    union FrameInfo {
        char value;
        struct {
            uint8_t size : 4;
            uint8_t type : 4;
        } data;
    };

    enum class FrameType {
        SingleFrame      = 0,
        FirstFrame       = 1,
        ConsecutiveFrame = 2,
        FlowControl      = 3
    };

    struct Frame {
        FrameType type;
        uint8_t size;

        Frame(std::array<uint8_t, 8>);
    };
}
