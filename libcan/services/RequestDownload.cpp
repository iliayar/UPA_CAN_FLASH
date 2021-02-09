#define SERVICE RequestDownload
#define REQUEST_ID 0x34
#define RESPONSE_ID 0x74

// If Service has subfunction, there is must be SUBFUNCTION field
#define REQUEST_FIELDS (DataFormatIdentifier, data_format), (DataAndLengthFormatIdentifier, address_len_format), (std::vector<uint8_t>, memory_addr), (std::vector<uint8_t>, memory_size)
#define RESPONSE_FIELDS (std::shared_ptr<LengthFormatIdentifier>, length_format), (std::vector<uint8_t>, max_blocks_number)
// --- FIELD ---
// (<Type>, <Alias>)

SERVICE_BEGIN

// SUBFUNCTIONS( [(<subfunction name>, <8-bit value>)]... )

DATATYPE(DataFormatIdentifier, 
(INT, compressionMethod, 4),
(INT, encryptingMethod, 4))

DATATYPE(DataAndLengthFormatIdentifier,
(INT, memory_size, 4),
(INT, memory_address, 4))

DATATYPE(LengthFormatIdentifier,
(INT, memory_size, 4),
(INT, reserved, 4))

#ifdef EXTRA // Extra classes

#endif

// --- PARSE ---
// Avaliale vars: m_offset - number of red bits
// - FIELD(<TYPE>)
//   - ENUM(<New variable name>, <Enum class>, <Field length>)
//   - INT(<New variable name>, <Field length>)
//   - SUBFUNCTION {
//      [CASE(<subfunction>) {
//         break;
//        }]...
//     }
// - RETURN([Varible]...) - returns new response object
#ifdef PARSE // Parse Service Response
{
    FIELD(DATA, length_format, LengthFormatIdentifier);
    FIELD(VEC, max_blocks_number, length_format->get_memory_size()*8);
    RETURN(length_format, max_blocks_number);
}
#endif

// --- DUMP ---
// Avaliable fields: m_<Fields alias>
// - INIT
//   Avaliable variables:
//   - offset - number of written bits
//   - payload - result data
//   - writer - Writer class. Initialized from payload
// - FIELD(<TYPE>, ...)
//   - INT(<Value>, <Field length>)
//   - VEC(<Value>, <Field length>)
//   - SUBFUNCTION {
//      [CASE(<subfunction>) {
//         break;
//        }]...
//     }
// - RETURN - "return payload"
#ifdef DUMP // Dump Service Request to std::vector<uint8_t>
{
    INIT;
    FIELD(DATA, m_data_format);
    FIELD(DATA, m_address_len_format);
    FIELD(VEC, m_memory_addr, m_address_len_format.get_memory_address()*8);
    FIELD(VEC, m_memory_size, m_address_len_format.get_memory_size()*8);
    RETURN;
}
#endif

#undef SERVICE
#undef REQUEST_ID
#undef RESPONSE_ID
#undef REQUEST_FIELDS
#undef RESPONSE_FIELDS
