#define SERVICE WriteDataByIdentifier
#define REQUEST_ID 0x2e
#define RESPONSE_ID 0x6e
#define REQUEST_FIELDS (Data*, data)
#define RESPONSE_FIELDS (DataIdentifier, id)
SERVICE_BEGIN

#ifdef EXTRA

#endif

#ifdef PARSE
{
    FIELD(ENUM, id, DataIdentifier, 16);
    RETURN(id);
}
#endif


#ifdef DUMP
{
	INIT;
	FIELD(INT, m_data->get_type(), 16);
	FIELD(VEC, m_data->get_value(), m_data->get_value().size()*8);
	RETURN;
}
#endif

#undef SERVICE
#undef REQUEST_ID
#undef RESPONSE_ID
#undef REQUEST_FIELDS
#undef RESPONSE_FIELDS
