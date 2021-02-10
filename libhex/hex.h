#pragma once

#include <fstream>
#include <string>
#include <vector>

#include "map.h"

namespace Hex {

enum class HexLineType {
    Data = 0x00,
    EndOfFile = 0x01,
    ExtendSegmentAddress = 0x02,
    StartSegmentAddress = 0x03,
    ExtendLinearAddress = 0x04,
    StartLinearAddress = 0x05
};

class HexLine {
public:
    virtual HexLineType get_type() = 0;
};

#define HEXLINE_FIELD_GETTER(type, name) \
    type get_##name() { return m_##name; }
#define HEXLINE_FIELD(type, name) type m_##name;
#define HEXLINE_CTR_FIELD(type, name) type name
#define HEXLINE_CTR_FIELD_INIT(_, name) m_##name(name)
#define HEXLINE_CLASS_EMPTY(name)                            \
    class name##Line : public HexLine {                      \
    public:                                                  \
	name##Line() {}                                      \
	HexLineType get_type() { return HexLineType::name; } \
    }
#define HEXLINE_CLASS(name, ...)                                     \
    class name##Line : public HexLine {                              \
    public:                                                          \
	name##Line(MAP_TUPLE_LIST(HEXLINE_CTR_FIELD, __VA_ARGS__))   \
	    : MAP_TUPLE_LIST(HEXLINE_CTR_FIELD_INIT, __VA_ARGS__) {} \
	HexLineType get_type() { return HexLineType::name; }         \
	MAP_TUPLE(HEXLINE_FIELD_GETTER, __VA_ARGS__)                 \
    private:                                                         \
	MAP_TUPLE(HEXLINE_FIELD, __VA_ARGS__)                        \
    }

HEXLINE_CLASS(Data, (std::vector<uint8_t>, data), (uint16_t, address));
HEXLINE_CLASS_EMPTY(EndOfFile);
HEXLINE_CLASS(ExtendSegmentAddress, (uint16_t, address));
HEXLINE_CLASS(StartSegmentAddress, (uint32_t, address));
HEXLINE_CLASS(ExtendLinearAddress, (uint16_t, address));
HEXLINE_CLASS(StartLinearAddress, (uint32_t, address));
#undef HEXLINE_FIELD_GETTER
#undef HEXLINE_FIELD
#undef HEXLINE_CTR_FIELD
#undef HEXLINE_CTR_FIELD_INIT
#undef HEXLINE_CLASS
#undef HEXLINE_CLASS_EMPTY

class Source {
public:
    virtual char get_char() = 0;
    virtual void next_char() = 0;
    virtual bool is_eof() = 0;
};

class StringSource : public Source {
public:
    StringSource(std::string);
    char get_char();
    void next_char();
    bool is_eof();

private:
    std::string m_string;
    int m_pos;
};

class FileSource : public Source {
public:
    using filepath = std::string;
    FileSource(std::ifstream&);

    char get_char();
    void next_char();
    bool is_eof();

private:
    std::ifstream& m_fin;
    char* m_char;
};

class HexReader {
public:
    HexReader(Source* source);
    HexLine* read_line();
    bool is_eof();
    uint32_t get_current_address() { return m_address; }

private:
    void except(char);
    std::string read_chars(int);

    Source* m_source;
    uint32_t m_address = 0;

    int m_line_readed;
};

std::vector<uint8_t> str_to_bytes(std::string str);

}  // namespace Hex
