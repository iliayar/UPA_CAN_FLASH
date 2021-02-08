#define SERVICE DiagnosticSessionControl
#define REQUEST_ID 0x10
#define RESPONSE_ID 0x50

// If Service has subfunction, there is must be SUBFUNCTION field
#define REQUEST_FIELDS SUBFUNCTION
#define RESPONSE_FIELDS SUBFUNCTION
// --- FIELD ---
// (<Type>, <Alias>)

SERVICE_BEGIN

// SUBFUNCTIONS( [(<subfunction name>, <8-bit value>)]... )
SUBFUNCTIONS(
    (extendDiagnosticSession, 0x03),
    (programmingSession, 0x02)
    )

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
    FIELD(SUBFUNCTION) { }
    RETURN(subfunction);
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
    FIELD(SUBFUNCTION) { }
    RETURN;
}
#endif

#undef SERVICE
#undef REQUEST_ID
#undef RESPONSE_ID
#undef REQUEST_FIELDS
#undef RESPONSE_FIELDS
