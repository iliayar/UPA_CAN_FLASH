#define SERVICE RequestDownload
#define REQUEST_ID 0x34
#define RESPONSE_ID 0x74

// If Service has subfunction, there is must be SUBFUNCTION field
#define REQUEST_FIELDS (uint8_t, data_format), (uint8_t, address_len_format), (std::vector<uint8_t>, memory_addr), (std::vector<uint8_t>, memory_size)
#define RESPONSE_FIELDS (uint8_t, length_format), (std::vector<uint8_t>, max_blocks_number)
// --- FIELD ---
// (<Type>, <Alias>)

SERVICE_BEGIN

// SUBFUNCTIONS( [(<subfunction name>, <8-bit value>)]... )

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
    FIELD(INT, length_format, 8);
    FIELD(VEC, max_blocks_number, ALL);
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
    FIELD(INT, m_data_format, 8);
    FIELD(INT, m_address_len_format, 8);
    FIELD(VEC, m_memory_addr, m_memory_addr.size());
    FIELD(VEC, m_memory_size, m_memory_size.size());
    RETURN;
}
#endif

#undef SERVICE
#undef REQUEST_ID
#undef RESPONSE_ID
#undef REQUEST_FIELDS
#undef RESPONSE_FIELDS
