#define SERVICE SecurityAccess
#define REQUEST_ID 0x27
#define RESPONSE_ID 0x67

// 
#define REQUEST_FIELDS SUBFUNCTION, (INT, seed_par, 8), (INT, key, 32)
#define RESPONSE_FIELDS SUBFUNCTION, (INT, seed, 32)
// --- FIELD ---
// (<Type>, <Alias>)

SERVICE_BEGIN
SUBFUNCTIONS(
	(requestSeed, 0x03),
	(sendKey, 0x04),
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
	FIELD(SUBFUNCTION) {
		CASE(requestSeed) {
			FIELD(INT, seed, 32);
			RETURN(subfunction, seed);
		}
		CASE(sendKey) {
			RETURN(subfunction, 0);
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
//   - writer - Writer class. Initializrd from payload
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
	FIELD(SUBFUNCTION) {
		CASE(requestSeed) {
			FIELD(INT, m_seed_par, 8);
			break;
		}
		CASE(sendKey) {
			FIELD(INT, m_key, 32);
			break;
		}
	}
	RETURN;
}
#endif

#undef SERVICE
#undef REQUEST_ID
#undef RESPONSE_ID
#undef REQUEST_FIELDS
#undef RESPONSE_FIELDS
