#include "hex.h"

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

#include "bytes.h"
#include "crc.h"

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

bool Hex::HexReader::test(char c) {
    if (m_source->get_char() != c) {
        return false;
    }
    m_source->next_char();
    return true;
}
void Hex::HexReader::except(char c) {
    if (m_source->get_char() != c) {
        std::stringstream ss;
        ss << std::hex << (int)c << ", got " << (int)m_source->get_char();
        throw std::runtime_error("Invalid HEX format. Expect " + ss.str());
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

Hex::HexReader::HexReader(std::shared_ptr<Hex::Source> source)
    : m_source(std::move(source)), m_line_readed(0) {}

std::vector<uint8_t> sum_proxy(uint8_t* sum, std::vector<uint8_t> data) {
    for (uint8_t d : data) *sum += d;
    return data;
}

#define READ_INT(n)                                                \
    Util::Reader(sum_proxy(&sum, str_to_bytes(read_chars(n / 4)))) \
        .read_int<uint##n##_t>(n)

optional<std::shared_ptr<Hex::Line::Line>> Hex::HexReader::read_line() {
    except(':');

    uint8_t sum = 0;

    m_line_readed++;

    auto maybe_size = READ_INT(8);
    if(!maybe_size) {
        return {};
    }
    uint8_t size = maybe_size.value();
    auto maybe_address = READ_INT(16);
    if(!maybe_address) {
        return {};
    }
    uint16_t address = maybe_address.value();
    auto maybe_type = READ_INT(8);
    if(!maybe_type) {
        return {};
    }
    Hex::Line::Type type = static_cast<Line::Type>(maybe_type.value());

    optional<std::shared_ptr<Hex::Line::Line>> line = {};

    switch (type) {
        case Hex::Line::Type::Data: {
            std::vector<uint8_t> data =
                sum_proxy(&sum, str_to_bytes(read_chars(size * 2)));
            line = std::make_shared<Hex::Line::Data>(data, address);
            break;
        }
        case Hex::Line::Type::EndOfFile: {
            line = std::make_shared<Hex::Line::EndOfFile>();
            break;
        }
        case Hex::Line::Type::ExtendSegmentAddress: {
            auto maybe_addr = READ_INT(16);
            if(!maybe_addr) {
                return {};
            }
            line = std::make_shared<Hex::Line::ExtendLinearAddress>(maybe_addr.value());
            break;
        }
        case Hex::Line::Type::StartSegmentAddress: {
            auto maybe_addr = READ_INT(32);
            if(!maybe_addr) {
                return {};
            }
            line = std::make_shared<Hex::Line::StartSegmentAddress>(maybe_addr.value());
            break;
        }
        case Hex::Line::Type::ExtendLinearAddress: {
            auto maybe_addr = READ_INT(16);
            if(!maybe_addr) {
                return {};
            }
            line = std::make_shared<Hex::Line::ExtendLinearAddress>(maybe_addr.value());
            uint32_t addr_t = maybe_addr.value();
            m_address &= (uint32_t)0x0000ffff;
            m_address |= (addr_t << 16);
            break;
        }
        case Hex::Line::Type::StartLinearAddress: {
            auto maybe_addr = READ_INT(32);
            if(!maybe_addr) {
                return {};
            }
            line = std::make_shared<Hex::Line::StartLinearAddress>(maybe_addr.value());
            break;
        }
        default:
            throw std::runtime_error("Wrong HEX format: Invalid line type");
    }
    READ_INT(8);
    if (sum != 0) throw std::runtime_error("Wrong HEX format: Invalid sum");
    test('\r');
    except('\n');
    return line;
}

bool Hex::HexReader::is_eof() { return m_source->is_eof(); }

#undef READ_INT

Hex::FileSource::FileSource(std::ifstream& fin) : m_fin(fin), m_char(nullptr) {}

char Hex::FileSource::get_char() {
    if (m_char == nullptr) {
        m_char = new char();
        *m_char = m_fin.get();
    }
    return *m_char;
}

void Hex::FileSource::next_char() {
    if (m_char == nullptr) {
        m_char = new char();
    }
    *m_char = m_fin.get();
}

bool Hex::FileSource::is_eof() { return m_fin.eof(); }

optional<Hex::HexInfo> Hex::read_hex_info(Hex::HexReader& reader) {
    int size = 0;
    uint16_t crc = 0xffff;
    std::vector<uint8_t> last_4(4, 0);
    int last_4_i = 0;
    bool high_addr = false;
    bool low_addr = false;
    uint32_t addr = 0;
    while (!reader.is_eof()) {
        auto maybe_line = reader.read_line();
        if(!maybe_line) return {};
        auto line = maybe_line.value();
        if (line->get_type() == Hex::Line::Type::Data) {
            if (!low_addr) {
                low_addr = true;
                uint32_t l_addr =
                    std::static_pointer_cast<Hex::Line::Data>(line)->get_address();
                addr |= l_addr;
            }
            std::vector<uint8_t> line_data =
                std::static_pointer_cast<Hex::Line::Data>(line)->get_data();
            for (uint8_t d : line_data) {
                last_4[last_4_i++] = d;
                size++;
                if (last_4_i >= 4) {
                    crc = Util::crc16_block(last_4, crc);
                    last_4_i = 0;
                }
            }
        }
        if (!high_addr &&
            line->get_type() == Hex::Line::Type::ExtendLinearAddress) {
            high_addr = true;
            uint32_t h_addr =
                std::static_pointer_cast<Hex::Line::ExtendLinearAddress>(line)
                    ->get_address();
            addr |= (h_addr << 16);
        }
        if (line->get_type() == Hex::Line::Type::EndOfFile) {
            break;
        }
    }
    if (last_4_i != 0) {
        while (last_4_i < 4) last_4[last_4_i++] = 0;
        crc = Util::crc16_block(last_4, crc);
    }
    return HexInfo{addr, size, crc};
}
