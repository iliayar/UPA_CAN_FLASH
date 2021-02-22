/**
 *  --- FRAME --- 
 *  Frame Types
 *  --------------
 *  <Alias>, <4 Bit value>, [FIELD]...
 *  --------------

 *  ------ FIELD ------
 *  Define fields in frame
 *  <INT|VEC|ENUM>(Frame type), <Field type args>...
 *  --- Field types ---
 *  - INT - One number value
 *    <field alias>, <Bit len of field>
 *  - VEC - Array of bytes
 *    <field alias>, <Bit len of field|ALL>(ALL - the rest of bits)
 *  - ENUM - Predefined enumeration
 *    <field alias>, <Predefined enum class name>, <Bit len of field>
 *  -------------------
 */
FRAME(SingleFrame, 0x00,
(INT, len, 4),
(VEC, data, ALL))

FRAME(FirstFrame, 0x01,
(INT, len, 12),
(VEC, data, ALL))

FRAME(ConsecutiveFrame, 0x02,
(INT, seq_num, 4),
(VEC, data, ALL))

FRAME(FlowControl, 0x03,
(ENUM, status, FlowStatus, 4),
(INT, block_size, 8),
(INT, min_separation_time, 8))