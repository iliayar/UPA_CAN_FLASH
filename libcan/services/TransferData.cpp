#define SERVICE TransferData
#define REQUEST_ID 0x36
#define RESPONSE_ID 0x76

// If Service has subfunction, there is must be SUBFUNCTION field
#define REQUEST_FIELDS (uint8_t, block_counter), (std::vector<uint8_t>, data)
#define RESPONSE_FIELDS (uint8_t, block_counter), (std::vector<uint8_t>, data)
// --- FIELD ---
// (<Type>, <Alias>)

SERVICE_BEGIN

// [SUBFUNCTIONS( [(<subfunction name>, <8-bit value>)]... )]
// [DATATYPE( [(<field type>, <field name>)]... )]...

#ifdef EXTRA // Extra classes

#endif

// --- PARSE ---
// Avaliale vars: m_offset - number of red bits
// - FIELD(<TYPE>)
//   - ENUM(<New variable name>, <Enum class>, <Field length>)
//   - INT(<New variable name>, <Field length>)
//   - VEC(<New variable name>, <Field length>)
//   - DATATYPE(<New variable name>, <Data class>)
//   - SUBFUNCTION {
//      [CASE(<subfunction>) {
//         break;
//        }]...
//     }
// - RETURN([Varible]...) - returns new response object
#ifdef PARSE // Parse Service Response
{
    FIELD(INT, block_counter, 8);
    FIELD(VEC, data, ALL);
    RETURN(block_counter, data);
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
//   - DATATYPE(<Value>)
//   - SUBFUNCTION {
//      [CASE(<subfunction>) {
//         break;
//        }]...
//     }
// - RETURN - "return payload"
#ifdef DUMP // Dump Service Request to std::vector<uint8_t>
{
    INIT;
    FIELD(INT, m_block_counter, 8);
    FIELD(VEC, m_data, m_data.size()*8);
    RETURN;
}
#endif

#undef SERVICE
#undef REQUEST_ID
#undef RESPONSE_ID
#undef REQUEST_FIELDS
#undef RESPONSE_FIELDS
