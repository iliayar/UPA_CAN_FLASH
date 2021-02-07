
// --- FRAMES --- 
// enum for frame types
// --------------
// <Alias>, <8 Bit value>

FRAMES(
(SingleFrame, 0x00),
(FirstFrame, 0x01),
(ConsecutiveFrame, 0x02),
(FlowControl,  0x03)
)

// --- FRAME_CLASS ---
// class for frame type
// -------------------
// <FrameType>, [<Frame field C++ type>, <Frame field alias>]...

FRAME_CLASS(SingleFrame,
(int, len),
(ARRAY, data))

FRAME_CLASS(FirstFrame,
(int, len),
(ARRAY, data))

FRAME_CLASS(ConsecutiveFrame,
(int, seq_num),
(ARRAY, data))

FRAME_CLASS(FlowControl,
(FlowStatus, status),
(int, block_size),
(int, min_separation_time))


// --- PARSE ---
// Parse frame for raw bytes
// -------------
// <FrameType>, [<Field description>]
// --- Field description ---
// - INT - One number value
//   <field alias>, <8|16|32|64>(Parse result, i.e uintN_t), <Bit len of field>
// - VEC - Array of bytes
//   <field alias>, <Bit len of field>
// - ENUM - Predefined enumeration
//   <field alias>, <Predefined enum name>, <Same as INT:2>, <Same as INT:3>
// -------------
// Order matters, fields defined before can be used
// m_offset = "The number of already red bits"

PARSE(SingleFrame,
(INT, len, 8, 4),
(VEC, data, len * 8))

PARSE(FirstFrame,
(INT, len, 16, 12),
(VEC, data, 64 - m_offset))

PARSE(ConsecutiveFrame,
(INT, seq_num, 8, 4),
(VEC, data, 64 - m_offset))

PARSE(FlowControl,
(ENUM, status, FlowStatus, 8, 4),
(INT, block_size, 8, 8),
(INT, min_separation_time, 8, 8))

// --- DUMP ---
// Dump frame for raw bytes
// ------------
// <FrameType>, [<Field description>]
// --- Field description ---
// - INT - One number value
//   <field alias>, <8|16|32|64>(Parse result, i.e uintN_t), <Bit len of field>
// - VEC - Array of bytes
//   <field alias>, <Bit len of field>
// -------------
// Predefined enums have to be written as INT
// offset = "The number of already written bits"

DUMP(SingleFrame,
(INT, len, 8, 4),
(VEC, data, m_len * 8))

DUMP(FirstFrame,
(INT, len, 16, 12),
(VEC, data, 64 - offset))

DUMP(ConsecutiveFrame,
(INT, seq_num, 8, 4),
(VEC, data, 64 - offset))

DUMP(FlowControl,
(INT, status, 8, 4),
(INT, block_size, 8, 8),
(INT, min_separation_time, 8, 8))