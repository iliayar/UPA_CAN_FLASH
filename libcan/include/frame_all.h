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
    class Builder : public Util::Builder<SingleFrame, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader const& reader) : B() {
            read(reader, object()->m_len, object()->m_data);
        }
        auto len(uint8_t len) { return field(object()->m_len, len); }
        auto data(std::vector<uint8_t> const& data) {
            return field(object()->m_data, data);
        }
    };
    uint8_t get_len() { return m_len.get().value(); }
    std::vector<uint8_t> get_data() { return m_data.get().value(); }
    Type get_type() { return m_type.get().value(); }

    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader const& reader) {
        return std::make_unique<Builder>(reader);
    }

protected:
    bool dump_impl(Util::Writer& writer) {
        return Util::write_args(writer, m_len, m_data);
    }

private:
    Util::EnumField<Type, uint8_t, 4> m_type = Type::SingleFrame;
    Util::IntField<uint8_t, 4> m_len;
    Util::VecField<64 - 4 - 4> m_data;
};

class FirstFrame : public Frame {
public:
    class Builder : public Util::Builder<FirstFrame, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader const& reader) : B() {
            read(reader, object()->m_len, object()->m_data);
        }
        auto len(uint16_t len) { return field(object()->m_len, len); }
        auto data(std::vector<uint8_t> const& data) {
            return field(object()->m_data, data);
        }
    };
    auto get_len() { return m_len.get().value(); }
    auto get_data() { return m_data.get().value(); }
    Type get_type() { return m_type.get().value(); }

    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader const& reader) {
        return std::make_unique<Builder>(reader);
    }

protected:
    bool dump_impl(Util::Writer& writer) {
        return Util::write_args(writer, m_len, m_data);
    }

private:
    Util::EnumField<Type, uint8_t, 4> m_type = Type::FirstFrame;
    Util::IntField<uint16_t, 12> m_len;
    Util::VecField<64 - 12 - 4> m_data;
};

class ConsecutiveFrame : public Frame {
public:
    class Builder : public Util::Builder<ConsecutiveFrame, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader const& reader) : B() {
            read(reader, object()->m_seq_num, object()->m_data);
        }
        auto seq_num(uint8_t seq_num) {
            return field(object()->m_seq_num, seq_num);
        }
        auto data(std::vector<uint8_t> const& data) {
            return field(object()->m_data, data);
        }
    };
    uint8_t get_seq_num() { return m_seq_num.get().value(); }
    std::vector<uint8_t> get_data() { return m_data.get().value(); }
    Type get_type() { return m_type.get().value(); }

    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader const& reader) {
        return std::make_unique<Builder>(reader);
    }

protected:
    bool dump_impl(Util::Writer& writer) {
        return Util::write_args(writer, m_seq_num, m_data);
    }

private:
    Util::EnumField<Type, uint8_t, 4> m_type = Type::ConsecutiveFrame;
    Util::IntField<uint8_t, 4> m_seq_num;
    Util::VecField<64 - 4 - 4> m_data;
};

class FlowControl : public Frame {
public:
    class Builder : public Util::Builder<FlowControl, Builder> {
    public:
        Builder() : B() {}
        Builder(Util::Reader const& reader) : B() {
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
    };
    FlowStatus get_status() { return m_status.get().value(); }
    uint8_t get_block_size() { return m_block_size.get().value(); }
    uint8_t get_min_separation_time() {
        return m_min_separation_time.get().value();
    }
    Type get_type() { return m_type.get().value(); }

    static std::unique_ptr<Builder> build() {
        return std::make_unique<Builder>();
    }
    static std::unique_ptr<Builder> build(Util::Reader const& reader) {
        return std::make_unique<Builder>(reader);
    }

protected:
    bool dump_impl(Util::Writer& writer) {
        return Util::write_args(writer, m_status, m_block_size, m_min_separation_time);
    }

private:
    Util::EnumField<Type, uint8_t, 4> m_type = Type::FlowControl;
    Util::EnumField<FlowStatus, uint8_t, 4> m_status;
    Util::IntField<uint8_t, 8> m_block_size;
    Util::IntField<uint8_t, 8> m_min_separation_time;
};

}  // namespace Frame
}  // namespace Can
