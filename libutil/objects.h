/**
 * @file objects.h
 * Implement classes desribing abstract fields(for requests and frames)
 */
#pragma once

#include "bytes.h"

namespace Util {
    /**
     * Base class of field. Can be any size, provided in
     * constructor. Controls field state, when accessing, checks if
     * field is initialized.
     *
     * @tparam T The type of field value. Must
     * be copyable and assignable
     */
template <typename T>
struct VarField {

    /**
     * @param value
     * @param size The size of field in bits 
     */
    VarField(T value, int size) : m_size(size), m_value(value), m_valid(true) {}
    VarField(int size) : m_size(size), m_valid(false) {}
    VarField() : m_size(0), m_valid(false) {}

    /**
     * Write this field to provide writer. The write behaviour
     * implements in derived classes with method {@link #write_impl}
     *
     * @param writer
     * @return false if this field is not initialized or writer failed, true otherwise
     */
    bool write(Writer& writer) {
        if (!m_valid) return false;
        return write_impl(writer, m_value);
    }


    /**
     * Reads to this field from writer. The read behaviour implements
     * in deriving classes through method {@link #read_impl(Reader&)}. If read
     * was success this field become initilized
     *
     * @param reader
     */
    void read(Reader& reader) {
        auto res = read_impl(reader);
        if (res) {
            m_value = res.value();
            m_valid = true;
        } else {
            m_valid = false;
        }
    }

    /**
     * Try get value, stored in this field
     * 
     * @return Nothing if this field is not initialized, Some(value) otherwise
     */
    optional<T> get() {
        if (m_valid) {
            return m_value;
        } else {
            return {};
        }
    }

    /**
     * Initialize this field with provided value.
     *
     * @param value
     * @return field reference
     * @see #set(T)
     */
    VarField<T>& operator=(const T& value) {
        m_value = value;
        m_valid = true;
        return *this;
    }

    /**
     * Inialize this field with provided value. After this operaion this field become initialized.
     *
     * @param value
     */
    void set(T value) {
        m_value = value;
        m_valid = true;
    }

    /**
     * Make this Field to read the rest of values in reader
     * It reads by one byte, so the bytes count must be integer.
     */
    void all() {
        m_all = true;
    }

    virtual int get_size() { return m_size; }

    /**
     * @return true if this field is initialized, false otherwise
     */
    bool valid() { return m_valid; }

protected:
    virtual optional<T> read_impl(Reader& reader) = 0;
    virtual bool write_impl(Writer& writer, T value) = 0;

    int m_size = 0;
    T m_value;
    bool m_valid = false;
    bool m_all = false;
};

/**
 * Abstract field class with fixed size.
 * @param T The value type
 * @param size The size of field in bits
 * @see VarField<T>
 */
template <typename T, int size>
struct Field : VarField<T> {
    Field(T value) : VarField<T>(value, size) {}
    Field() : VarField<T>(size) {}
};

/**
 * The field, contains integer value. Has fixed size
 * @tparam T The integer type
 * @tparam size The size of integer value in bits
 */
template <typename T, int size>
struct IntField : public Field<T, size> {
    IntField(T value) : Field<T, size>(value) {}
    IntField() : Field<T, size>() {}

protected:
    bool write_impl(Writer& writer, T value) {
        return writer.write_int<T>(value, size);
    }
    optional<T> read_impl(Reader& reader) { return reader.read_int<T>(size); }
};

/**
 * Field for an arbitrary number of bytes. Resizable
 */
struct VarVecField : public VarField<std::vector<uint8_t>> {
    VarVecField(std::vector<uint8_t> data, int size)
        : VarField<std::vector<uint8_t>>(data, size) {}
    VarVecField(int size) : VarField<std::vector<uint8_t>>(size) {}
    VarVecField() : VarField<std::vector<uint8_t>>() {}

    VarVecField& operator=(std::vector<uint8_t> value) {
        m_value = value;
        m_size = value.size()*8;
        m_valid = true;
        return *this;
    }

    void resize(int size) {
        m_size = size;
        if (m_value.size() < size) m_value.resize(size);
    }

protected:
    bool write_impl(Writer& writer, std::vector<uint8_t> value) {
        return writer.write(value, m_size);
    }
    optional<std::vector<uint8_t>> read_impl(Reader& reader) {
        if(m_all) {
            std::vector<uint8_t> res{};
            while(!reader.is_eof()) {
                res.push_back(reader.read_int<uint8_t>(8).value());
            }
            return res;
        }
        return reader.read(m_size);
    }
};

/**
 * Field for an fixed number of bies.
 * @tparam size The size of all bits
 */
template <int size>
struct VecField : public VarVecField {
public:
    VecField(std::vector<uint8_t> data) : VarVecField(data, size) {}
    VecField() : VarVecField(size) {}
};

/**
 * Field for numerations.
 * @tparam T The enumeration type
 * @tparam I The integer type appropriate to enumeration type
 * @tparam size The size of integer value in bits
 */
template <typename T, typename I, int size>
struct EnumField : public Field<T, size> {
    EnumField(T value) : Field<T, size>(value) {}
    EnumField() : Field<T, size>() {}

    EnumField<T, I, size>& operator=(T value) {
        this->m_value = value;
        this->m_valid = true;
        return *this;
    }

    EnumField<T, I, size>& operator=(I value) {
        this->m_value = static_cast<T>(value);
        this->m_valid = true;
        return *this;
    }

protected:
    bool write_impl(Writer& writer, T value) {
        return writer.write_int<I>(static_cast<I>(value), size);
    }
    optional<T> read_impl(Reader& reader) {
        auto res = reader.read_int<I>(size);
        if (res) {
            return static_cast<T>(res.value());
        } else {
            return {};
        }
    }
};

/**
 * {@c get_size()} method always returns {@c 0}
 * @brief Field represents custom class
 * @tparam T must contains {@c write(Util::Writer&)} method, provide static
 * method {@c build()}, wich returns {@c Builder}, that's also implements {@c
 * build()}, which returns {@c std::shared_ptr<T>} or {@c
 * optional<std::shared_ptr<T>>}
 */
template <typename T>
struct DataField : public VarField<std::shared_ptr<T>> {
public:
    DataField(T* ptr)
        : VarField<std::shared_ptr<T>>(std::shared_ptr<T>(ptr), 0) {}
    DataField() : VarField<std::shared_ptr<T>>(0) {}

    DataField<T>& operator=(std::shared_ptr<T> value) {
        this->m_value = value;
        this->m_valid = true;
        return *this;
    }

protected:
    bool write_impl(Writer& writer, std::shared_ptr<T> value) {
        return value->write(writer);
    }
    optional<std::shared_ptr<T>> read_impl(Reader& reader) {
        return T::build(reader)->build();
    }
};

template <typename... F>
bool write_args(Writer& writer, F&... args) {
    bool res = true;
    over_all<F&...>::for_each([&](auto& field) { res &= field.write(writer); },
                              std::forward<F&>(args)...);
    return res;
}

template <typename... F>
optional<std::vector<uint8_t>> dump_args(F&... args) {
    DynamicWriter writer{};
    if (!write_args(writer, std::forward<F&>(args)...)) {
        return {};
    }
    return writer.get_payload();
}

template <typename... F>
bool read_args(Reader& reader, F&... args) {
    bool res = true;
    over_all<F&...>::for_each(
        [&](auto& field) {
            field.read(reader);
            res &= field.valid();
        },
        std::forward<F&>(args)...);
    return res;
}

/**
 * Abstract builder class for objects like request/response, frames
 * @tparam R The class type for which this builder is declared
 * @tparam Self The declraed builder class. FIXME It must be deleted somehow
 */
template <typename R, class Self>
class Builder {
public:
    using B = Util::Builder<R, Self>;
    Builder() { m_object = std::make_shared<R>(); }

    /**
     * Build the entire object.
     *
     * @return Nothing if there was an error during building, Some(object) otherwise
     */
    optional<std::shared_ptr<R>> build() {
        if (!m_valid) {
            return {};
        }
        return m_object;
    }

protected:
    /**
     * Read provided fields from reader
     * {@code read(reader, object()->field1, object()->field2)}
     */
    template <typename... F>
    void read(Reader& reader, F&... fields) {
        if (!read_args(reader, std::forward<F&>(fields)...)) {
            fail();
        }
    }

    /**
     * Accessing the buildable object
     * @return The pointer to object
     */
    std::shared_ptr<R> object() { return m_object; }

    /**
     * Initialize provide field with value
     * {@code field(object()->field1, value)}
     */
    template <typename F, typename T>
    Self* field(F& field, T value) {
        field = value;
        return static_cast<Self*>(this);
    }

    /**
     * Make the whole build process invalid.
     */
    void fail() { m_valid = false; }

private:
    std::shared_ptr<R> m_object;
    bool m_valid = true;
};

}  // namespace Util
