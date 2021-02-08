#define SERVICE // <Service Name>
#define REQUEST_ID // <Service Request 8 bit identifier>
#define RESPONSE_ID // <Service Response 8 bit indentifier>

#define REQUEST_FIELDS // [FIELD]...
#define RESPONSE_FIELDS // [FIELD]...
// --- FIELD ---
// (<Type>, <Alias>)

SERVICE_BEGIN

#ifdef EXTRA // Extra classes

#endif

// --- PARSE ---
// Avaliale vars: m_offset - number of red bits
// - FIELD(<TYPE>)
//   - ENUM(<New variable name>, <Enum class>, <Field length>)
// - RETURN([Varible]...) - returns new request object
#ifdef PARSE // Parse Service Response
{
	// ...
}
#endif

// --- DUMP ---
// Avaliable fields: m_<Fields alias>
// - FIELD(<TYPE>, ...)
//   - INTN(<Value>, <Number type to dumo field in>(uintN_t), <Field length>)
//   - VEC(<Value>, <Field length>)
// - INIT(<Total length of dump>)
//   Avaliable variables:
//   - offset - number of written bits
//   - payload - result data
//   - writer - Writer class. Initializrd from payload
// - RETURN - "return payload"
#ifdef DUMP // Dump Service Request to std::vector<uint8_t>
{
	// ...
}
#endif

#undef SERVICE
#undef REQUEST_ID
#undef RESPONSE_ID
#undef REQUEST_FIELDS
#undef RESPONSE_FIELDS
