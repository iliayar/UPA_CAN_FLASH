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

#include <experimental/optional>

using std::experimental::optional;

namespace Hex {


/**
 * Generates HEX line classes
 */

namespace Line {
enum class Type {
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
class Line {
public:
    /**
     * @return type of single HEX file line
     */
    virtual Type get_type() = 0;
};
class Data : public Line {
public:
    Data(std::vector<uint8_t> data, uint16_t address)
        : m_data(data), m_address(address) {}
    Type get_type() override { return Type::Data; }
    std::vector<uint8_t> get_data() { return m_data; }
    uint16_t get_address() { return m_address; }

private:
    std::vector<uint8_t> m_data;
    uint16_t m_address;
};

class EndOfFile : public Line {
public:
    EndOfFile() {}
    Type get_type() override { return Type::EndOfFile; }
};

class ExtendSegmentAddress : public Line {
public:
    ExtendSegmentAddress(uint16_t address)
        : m_address(address) {}
    Type get_type() override { return Type::ExtendSegmentAddress; }
    uint16_t get_address() { return m_address; }

private:
    uint16_t m_address;
};

class StartSegmentAddress : public Line {
public:
    StartSegmentAddress(uint32_t address)
        : m_address(address) {}
    Type get_type() override { return Type::ExtendSegmentAddress; }
    uint32_t get_address() { return m_address; }

private:
    uint32_t m_address;
};

class ExtendLinearAddress : public Line {
public:
    ExtendLinearAddress(uint16_t address)
        : m_address(address) {}
    Type get_type() override { return Type::ExtendLinearAddress; }
    uint16_t get_address() { return m_address; }

private:
    uint16_t m_address;
};

class StartLinearAddress : public Line {
public:
    StartLinearAddress(uint32_t address)
        : m_address(address) {}
    Type get_type() override { return Type::StartLinearAddress; }
    uint32_t get_address() { return m_address; }

private:
    uint32_t m_address;
};

}  // namespace Line

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
    optional<std::shared_ptr<Line::Line>> read_line();

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
optional<HexInfo> read_hex_info(HexReader& reader);

std::vector<uint8_t> str_to_bytes(std::string str);

}  // namespace Hex
