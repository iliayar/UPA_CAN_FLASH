#define SERVICE RequestTransferExit
#define REQUEST_ID 0x37
#define RESPONSE_ID 0x77

// If Service has subfunction, there is must be SUBFUNCTION field
// --- FIELDS ---
// (TYPE, name, [type related params]...)
// --- TYPE ---
// - INT(<8|16|32|64>(bit length))
// - VEC() - array
// - DATA(<declared DATATYPE>)
// - RAW(<raw C++ type>)
#define REQUEST_FIELDS (INT, crc, 16)
#define RESPONSE_FIELDS (INT, crc, 16)
// --- FIELD ---
// (<Type>, <Alias>)

SERVICE_BEGIN

// [SUBFUNCTIONS( [(<subfunction name>, <8-bit value>)]... )]
// [DATATYPE( [(<field type>, <field name>)]... )]...

#ifdef EXTRA // Extra classes

#endif

// --- PARSE ---
// Avaliale vars: m_offset - number of red bits
// - FIELD(<TYPE>, <New variable name>, [<type related parameter>]...)
//   - ENUM(<Enum class>, <Field length>)
//   - INT(<Field length>)
//   - VEC(<Field length>)
//   - DATA(<Data class>)
//   - SUBFUNCTION {
//      [CASE(<subfunction>) {
//         break;
//        }]...
//     }
// - RETURN([Varible]...) - returns new response object
#ifdef PARSE // Parse Service Response
{
    if(m_size < 3*8) {
        RETURN(0);
    }
    FIELD(INT, crc, 16);
    RETURN(crc);
}
#endif

// --- DUMP ---
// Avaliable fields: m_<Fields alias>
// - INIT
//   Avaliable variables:
//   - offset - number of written bits
//   - payload - result data
//   - writer - Writer class. Initialized from payload
// - FIELD(<TYPE>, <value>, [<type related parameter>]...)
//   - INT(<Field length>)
//   - VEC(<Field length>)
//   - DATA()
//   - SUBFUNCTION {
//      [CASE(<subfunction>) {
//         break;
//        }]...
//     }
// - RETURN - "return payload"
#ifdef DUMP // Dump Service Request to std::vector<uint8_t>
{
    INIT;
    FIELD(INT, m_crc, 16);
    RETURN;
}
#endif

#undef SERVICE
#undef REQUEST_ID
#undef RESPONSE_ID
#undef REQUEST_FIELDS
#undef RESPONSE_FIELDS
