/**
 * @file hex.h
 * Defines class for parsing Intel HEX
 * format files
 */ 
#pragma once

#include <fstream>
#include <string>
#include <vector>
#include <memory>

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

/**
 * Class represent single line in HEX file
 */
class HexLine {
public:

    /**
     * @return type of single HEX file line
     */
    virtual HexLineType get_type() = 0;
};

/**
 * Generates HEX line classes
 */
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

/**
 * @interface
 * Provide methods to access any source
 * of HEX file data
 */
class Source {
public:

    /**
     * @return single char at current position
     */
    virtual char get_char() = 0;

    /**
     * Makes a single step to the next byte of HEX file data
     */
    virtual void next_char() = 0;


    /**
     * @return true is there is an EOF reached
     */
    virtual bool is_eof() = 0;
};


/**
 * Implementation pf Source interface
 * On standart C++ string
 */
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

/**
 * Implementation pf Source interface
 * On standart C++ file stream
 */
class FileSource : public Source {
public:
    using filepath = std::string;
    FileSource(std::ifstream&);

    char get_char();
    void next_char();
    bool is_eof();

    ~FileSource() {
        if(m_char != nullptr) delete m_char;
    }

private:
    std::ifstream& m_fin;
    char* m_char;
};

/**
 * Class to read HEX file
 */
class HexReader {
public:

    /**
     * @param Source interface class to get bytes from
     */
    HexReader(std::shared_ptr<Source> source);

    /**
     * @return next line of HEX file
     */
    std::shared_ptr<HexLine> read_line();

    /**
     * @return true if there is and EOF of provided Source
     */
    bool is_eof();

    /**
     * @return 4 bytes address calculated from
     * previous lines addresses
     */
    uint32_t get_current_address() { return m_address; }

private:
    void except(char);
    bool test(char);
    std::string read_chars(int);

    std::shared_ptr<Source> m_source;
    uint32_t m_address = 0;

    int m_line_readed;
};

/**
 * HEX file info
 * Starting address, size, and CRC sum of file
 */
struct HexInfo {
    uint64_t start_addr;
    int size;
    uint16_t crc;
};

/**
 * @param HEX file reader
 * @return HEX file info
 */
HexInfo read_hex_info(HexReader& reader);

std::vector<uint8_t> str_to_bytes(std::string str);

}  // namespace Hex
