#pragma once

#include "can.h"

#include <cstdint>
#include <vector>

namespace Can {

    enum class FrameType {
        SingleFrame      = 0,
        FirstFrame       = 1,
        ConsecutiveFrame = 2,
        FlowControl      = 3
    };

    class Frame {
    public:
        virtual FrameType get_type() = 0;
    };

    class Frame_SingleFrame : public Frame {
    public:
        Frame_SingleFrame(std::vector<uint8_t>, int);
        
        FrameType get_type() {
            return FrameType::SingleFrame;
        }

        std::vector<uint8_t> get_data() { return m_data; }
        int get_len() { return m_len; }

    private:
        std::vector<uint8_t> m_data;
        int m_len;
    };

    class Frame_FirstFrame : public Frame {
    public:
        Frame_FirstFrame(std::vector<uint8_t>, int);
        
        FrameType get_type() {
            return FrameType::FirstFrame;
        }

        std::vector<uint8_t> get_data() { return m_data; }
        int get_len() { return m_len; }

    private:
        std::vector<uint8_t> m_data;
        int m_len;
    };

    class Frame_ConsecutiveFrame : public Frame {
    public:
        Frame_ConsecutiveFrame(std::vector<uint8_t>, int);
        
        FrameType get_type() {
            return FrameType::ConsecutiveFrame;
        }
        
        std::vector<uint8_t> get_data() { return m_data; }
        int get_seq_num() { return m_seq_num; }

    private:
        std::vector<uint8_t> m_data;
        int m_seq_num;
    };


    enum class FlowStatus {
        ContinueToSend = 0,
        WaitForAnotherFlowControlMessageBeforeContinuing = 1,
        OverflowAbortTransmission = 2
    };
    
    class Frame_FlowControl : public Frame {
    public:
        Frame_FlowControl(FlowStatus, int, int);
        
        FrameType get_type() {
            return FrameType::FlowControl;
        }

        FlowStatus get_status() { return m_status; }
        int get_block_size() { return m_block_size; }
        int get_min_separation_time() { return m_min_separation_time; }
        
    private:
        FlowStatus m_status;
        int m_block_size;
        int m_min_separation_time;
    };

    class FrameFactory {
    public:
        FrameFactory(std::vector<uint8_t>);

        Frame* get();
    private:
        Frame* parse_SingleFrame();
        Frame* parse_FirstFrame();
        Frame* parse_ConsecutiveFrame();
        Frame* parse_FlowControl();

        int m_offset;
        Reader m_reader;
    };
}
