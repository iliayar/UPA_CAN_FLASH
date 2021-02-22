#define SERVICE ReadDataByIdentifier
#define REQUEST_ID 0x22
#define RESPONSE_ID 0x62
#define REQUEST_FIELDS (RAW, id, DataIdentifier)
#define RESPONSE_FIELDS (RAW, data, Data*)
SERVICE_BEGIN
#ifdef EXTRA  // {{{

#endif  // }}}

#ifdef PARSE  // {{{
{
    m_reader.add_offset(m_offset);

    Can::Data* data = Can::DataFactory(m_reader).get();

    RETURN(data);
}
#endif  // }}}

#ifdef DUMP  // {{{
{
    INIT;
    FIELD(INT, m_id, 16);
    RETURN;
}
#endif  // }}}

#undef SERVICE
#undef REQUEST_ID
#undef RESPONSE_ID
#undef REQUEST_FIELDS
#undef RESPONSE_FIELDS
