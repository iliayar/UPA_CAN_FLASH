/**
 * @file frame_all.h
 * Here defined classes of frame types
 * contains in {{ frame.inl }}
 */
#pragma once

#include <cstdint>
#include <vector>

#include "bytes.h"
#include "frame.h"
#include "objects.h"

namespace Can {

namespace Frame {
enum class FlowStatus {
    ContinueToSend = 0,
    WaitForAnotherFlowControlMessageBeforeContinuing = 1,
    OverflowAbortTransmission = 2
};

class SingleFrame : public Frame {
public:
    class Builder : public Util::Builder<SingleFrame> {
    public:
        Builder() : Util::Builder<SingleFrame>() {}
        Builder(Util::Reader& reader) : Util::Builder<SingleFrame>() {
            read(reader, object()->m_len, object()->m_data);
        }
        auto len(uint8_t len) { return field(object()->m_len, len); }
        auto data(std::vector<uint8_t> data) {
            return field(object()->m_data, data);
        }

    protected:
        using Self = Builder;
    };
    FrameType get_type() { return FrameType::SingleFrame; }
    std::vector<uint8_t> dump() { return Util::dump_args(m_len, m_data); }

    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }

private:
    Util::IntField<uint8_t, 4> m_len;
    Util::VecField<64 - 4> m_data;
};

class FirstFrame : public Frame {
public:
    class Builder : public Util::Builder<FirstFrame> {
    public:
        Builder() : Util::Builder<FirstFrame>() {}
        Builder(Util::Reader& reader) : Util::Builder<FirstFrame>() {
            read(reader, object()->m_len, object()->m_data);
        }
        auto len(uint8_t len) { return field(object()->m_len, len); }
        auto data(std::vector<uint8_t> data) {
            return field(object()->m_data, data);
        }

    protected:
        using Self = Builder;
    };
    FrameType get_type() { return FrameType::FirstFrame; }
    std::vector<uint8_t> dump() { return Util::dump_args(m_len, m_data); }

    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }

private:
    Util::IntField<uint16_t, 12> m_len;
    Util::VecField<64 - 12> m_data;
};

class ConsecutiveFrame : public Frame {
public:
    class Builder : public Util::Builder<ConsecutiveFrame> {
    public:
        Builder() : Util::Builder<ConsecutiveFrame>() {}
        Builder(Util::Reader& reader) : Util::Builder<ConsecutiveFrame>() {
            read(reader, object()->m_seq_num, object()->m_data);
        }
        auto seq_num(uint8_t seq_num) {
            return field(object()->m_seq_num, seq_num);
        }
        auto data(std::vector<uint8_t> data) {
            return field(object()->m_data, data);
        }

    protected:
        using Self = Builder;
    };
    FrameType get_type() { return FrameType::ConsecutiveFrame; }
    std::vector<uint8_t> dump() { return Util::dump_args(m_seq_num, m_data); }

    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }

private:
    Util::IntField<uint8_t, 4> m_seq_num;
    Util::VecField<64 - 4> m_data;
};

class FlowControl : public Frame {
public:
    class Builder : public Util::Builder<FlowControl> {
    public:
        Builder() : Util::Builder<FlowControl>() {}
        Builder(Util::Reader& reader) : Util::Builder<FlowControl>() {
            read(reader, object()->m_status, object()->m_block_size,
                 object()->m_min_separation_time);
        }
        auto status(FlowStatus status) {
            return field(object()->m_status, status);
        }
        auto block_size(int block_size) {
            return field(object()->m_block_size, block_size);
        }
        auto min_separation_time(int min_separation_time) {
            return field(object()->m_min_separation_time, min_separation_time);
        }

    protected:
        using Self = Builder;
    };
    FlowControl(Util::Reader& reader, bool& ok) {
        ok = Util::read_args(reader, m_status, m_block_size,
                             m_min_separation_time);
    }
    FlowControl(FlowStatus status, int block_size, int min_separation_time)
        : m_status(status),
          m_block_size(block_size),
          m_min_separation_time(min_separation_time) {}
    FrameType get_type() { return FrameType::ConsecutiveFrame; }
    std::vector<uint8_t> dump() {
        return Util::dump_args(m_status, m_block_size, m_min_separation_time);
    }

    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader& reader) {
        return std::make_unique<Builder>(reader);
    }

private:
    Util::EnumField<FlowStatus, uint8_t, 4> m_status;
    Util::IntField<uint8_t, 8> m_block_size;
    Util::IntField<uint8_t, 8> m_min_separation_time;
};

}  // namespace Frame
}  // namespace Can
