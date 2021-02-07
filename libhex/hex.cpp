#include "hex.h"

#include <stdexcept>
#include <string>
#include <vector>

#include "bytes.h"

Hex::StringSource::StringSource(std::string string)
    : m_string(string), m_pos(0) {}

char Hex::StringSource::get_char() {
    if (m_pos < m_string.length()) {
	return m_string[m_pos];
    }
    throw std::runtime_error("EOF met");
}

void Hex::StringSource::next_char() { m_pos++; }

bool Hex::StringSource::is_eof() { return m_pos >= m_string.length(); }

std::vector<uint8_t> Hex::str_to_bytes(std::string str) {
    std::vector<uint8_t> res((str.length() + 1) / 2, 0);
    for (int i = 0; i < str.length(); ++i) {
	uint8_t cur;
	if (str[i] >= '0' && str[i] <= '9') {
	    cur = str[i] - '0';
	} else if (str[i] >= 'A' && str[i] <= 'F') {
	    cur = str[i] - 'A' + 10;
	} else if (str[i] >= 'a' && str[i] <= 'f') {
	    cur = str[i] - 'a' + 10;
	}
	res[i / 2] <<= (i % 2) * 4;
	res[i / 2] |= cur;
    }
    return res;
}

void Hex::HexReader::except(char c) {
    if (m_source->get_char() != c) {
	throw std::runtime_error("Invalid HEX format");
    }
    m_source->next_char();
}

std::string Hex::HexReader::read_chars(int n) {
    std::string str;
    for (int i = 0; i < n; ++i) {
	str += m_source->get_char();
	m_source->next_char();
    }
    return str;
}

Hex::HexReader::HexReader(Hex::Source* source) : m_source(source) {}

std::vector<uint8_t> sum_proxy(uint8_t* sum, std::vector<uint8_t> data) {
    for (uint8_t d : data) *sum += d;
    return data;
}

#define READ_INT(n)                                                \
    Util::Reader(sum_proxy(&sum, str_to_bytes(read_chars(n / 4)))) \
	.read_##n(0, n)

Hex::HexLine* Hex::HexReader::read_line() {
    except(':');

    uint8_t sum = 0;

    uint8_t size = READ_INT(8);
    uint16_t address = READ_INT(16);
    Hex::HexLineType type = static_cast<Hex::HexLineType>(READ_INT(8));

    Hex::HexLine* line = nullptr;

    switch (type) {
	case Hex::HexLineType::Data: {
	    std::vector<uint8_t> data =
		sum_proxy(&sum, str_to_bytes(read_chars(size * 2)));
	    line = new Hex::DataLine(data, address);
	    break;
	}
	case Hex::HexLineType::EndOfFile: {
	    line = new Hex::EndOfFileLine();
	    break;
	}
	case Hex::HexLineType::ExtendSegmentAddress: {
	    uint16_t addr = READ_INT(16);
	    line = new Hex::ExtendLinearAddressLine(addr);
	    break;
	}
	case Hex::HexLineType::StartSegmentAddress: {
	    uint32_t addr = READ_INT(32);
	    line = new Hex::StartSegmentAddressLine(addr);
	    break;
	}
	case Hex::HexLineType::ExtendLinearAddress: {
	    uint16_t addr = READ_INT(16);
	    line = new Hex::ExtendLinearAddressLine(addr);
	    break;
	}
	case Hex::HexLineType::StartLinearAddress: {
	    uint32_t addr = READ_INT(32);
	    line = new Hex::StartLinearAddressLine(addr);
	    break;
	}
	default:
	    throw std::runtime_error("Wrong HEX format: Invalid line type");
    }
    READ_INT(8);
    if (sum != 0) throw std::runtime_error("Wrong HEX format: Invalid sum");
    except('\n');
    return line;
}

bool Hex::HexReader::is_eof() { return m_source->is_eof(); }

#undef READ_INT
#undef CHECK_SUM

Hex::FileSource::FileSource(filepath path)
	: m_fin(path) {}

char Hex::FileSource::get_char() {
	if(m_char == nullptr) {
		m_char = new char();
		m_fin >> *m_char;
	}
	return *m_char;
}

void Hex::FileSource::next_char() {
	if(m_char == nullptr) {
		m_char = new char();
	}
	m_fin >> *m_char;
}

bool Hex::FileSource::is_eof() {
	return m_fin.eof();
}
