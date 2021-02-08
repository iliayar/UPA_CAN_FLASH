#ifdef EXTRA

enum class DataIdentifier { VIN = 0xf190, UPASystemType = 0x200e };

class Data {
public:
    Data(DataIdentifier type, std::vector<uint8_t> value)
	: m_type(type), m_value(value) {}

    DataIdentifier get_type() { return m_type; }
    std::vector<uint8_t> get_value() { return m_value; }

private:
    DataIdentifier m_type;
    std::vector<uint8_t> m_value;
};

class DataFactory {
public:
    DataFactory(std::vector<uint8_t> payload)
	: m_offset(0), m_reader(payload) {}

    DataFactory(Util::Reader reader) : m_offset(0), m_reader(reader) {}

    Data* get() {
	DataIdentifier type =
	    static_cast<DataIdentifier>(m_reader.read_16(m_offset, 16));
	m_offset += 16;

	switch (type) {
	    case DataIdentifier::VIN:
		return parse_VIN();
	    case DataIdentifier::UPASystemType:
		return parse_UPASystemType();
            default:
                return nullptr;
        }
    }

private:
#define PARSE_DATA(name, len)                                         \
    Data* parse_##name() {                                            \
	std::vector<uint8_t> data = m_reader.read(m_offset, len * 8); \
	m_offset += len * 8;                                          \
	m_reader.add_offset(m_offset);                                \
	return new Data(DataIdentifier::name, data);                  \
    }
    PARSE_DATA(VIN, 17)
    PARSE_DATA(UPASystemType, 1)
#undef PARSE_DATA

    int m_offset;
    Util::Reader m_reader;
};

#endif
