#define SERVICE ECUReset
#define REQUEST_ID 0x11
#define RESPONSE_ID 0x51

// If Service has subfunction, there is must be SUBFUNCTION field
// --- FIELDS ---
// (TYPE, name, [type related params]...)
// --- TYPE ---
// - INT(<8|16|32|64>(bit length))
// - VEC() - array
// - DATA(<declared DATATYPE>)
// - RAW(<raw C++ type>)
#define REQUEST_FIELDS SUBFUNCTION
#define RESPONSE_FIELDS SUBFUNCTION, (INT, power_down_timer, 8)
// --- FIELD ---
// (<Type>, <Alias>)

SERVICE_BEGIN

// [SUBFUNCTIONS( [(<subfunction name>, <8-bit value>)]... )]
// [DATATYPE( [(<field type>, <field name>)]... )]...
SUBFUNCTIONS(
    (hardReset, 0x01),
    (keyOffOnReset, 0x02),
    (softReset, 0x03),
    (enableRapidPowerShutDown, 0x04),
    (disableRapidPowerShutDown, 0x05)
    )

#ifdef EXTRA // Extra classes

#endif

// --- PARSE ---
// Avaliale vars: m_offset - number of red bits
// - FIELD(<TYPE>)
//   - ENUM(<New variable name>, <Enum class>, <Field length>)
//   - INT(<New variable name>, <Field length>)
//   - VEC(<New variable name>, <Field length>)
//   - DATA(<New variable name>, <Data class>)
//   - SUBFUNCTION {
//      [CASE(<subfunction>) {
//         break;
//        }]...
//     }
// - RETURN([Varible]...) - returns new response object
#ifdef PARSE // Parse Service Response
{
    FIELD(SUBFUNCTION) {
        CASE(enableRapidPowerShutDown) {
            FIELD(INT, power_down_timer, 8);
            RETURN(subfunction, power_down_timer);
        }
    }
    RETURN(subfunction, 0);
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
//   - DATA(<Value>)
//   - SUBFUNCTION {
//      [CASE(<subfunction>) {
//         break;
//        }]...
//     }
// - RETURN - "return payload"
#ifdef DUMP // Dump Service Request to std::vector<uint8_t>
{
    INIT;
    FIELD(SUBFUNCTION) {}
    RETURN;
}
#endif

#undef SERVICE
#undef REQUEST_ID
#undef RESPONSE_ID
#undef REQUEST_FIELDS
#undef RESPONSE_FIELDS
