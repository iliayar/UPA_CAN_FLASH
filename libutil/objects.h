#pragma once

#include "bytes.h"

namespace Util {
template <typename T>
struct VarField {
    VarField(T value, int size) : m_size(size), m_value(value), m_valid(true) {}
    VarField(int size) : m_size(size), m_valid(false) {}
    VarField() : m_size(0), m_valid(false) {}

    bool write(Writer& writer) {
        if (!m_valid) return false;
        return write_impl(writer, m_value);
    }
    void read(Reader& reader) {
        auto res = read_impl(reader);
        if (res) {
            m_value = res.value();
            m_valid = true;
        } else {
            m_valid = false;
        }
    }

    optional<T> get() {
        if (m_valid) {
            return m_value;
        } else {
            return {};
        }
    }

    VarField<T>& operator=(const T& value) {
        m_value = value;
        m_valid = true;
        return *this;
    }

    void set(T value) {
        m_value = value;
        m_valid = true;
    }

    virtual int get_size() { return m_size; }

    bool valid() { return m_valid; }

protected:
    virtual optional<T> read_impl(Reader& reader) = 0;
    virtual bool write_impl(Writer& writer, T value) = 0;

    int m_size = 0;
    T m_value;
    bool m_valid = false;
};

template <typename T, int size>
struct Field : VarField<T> {
    Field(T value) : VarField<T>(value, size) {}
    Field() : VarField<T>(size) {}
};

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

struct VarVecField : public VarField<std::vector<uint8_t>> {
    VarVecField(std::vector<uint8_t> data, int size)
        : VarField<std::vector<uint8_t>>(data, size) {}
    VarVecField(int size) : VarField<std::vector<uint8_t>>(size) {}
    VarVecField() : VarField<std::vector<uint8_t>>() {}

    VarVecField& operator=(std::vector<uint8_t> value) {
        m_value = value;
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
        return reader.read(m_size);
    }
};

template <int size>
struct VecField : public VarVecField {
public:
    VecField(std::vector<uint8_t> data) : VarVecField(data, size) {}
    VecField() : VarVecField(size) {}
};

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

template <typename R, class Self>
class Builder {
public:
    using B = Util::Builder<R, Self>;
    Builder() { m_object = std::make_shared<R>(); }

    optional<std::shared_ptr<R>> build() {
        if (!m_valid) {
            return {};
        }
        return m_object;
    }

protected:
    template <typename... F>
    void read(Reader& reader, F&... fields) {
        if (!read_args(reader, std::forward<F&>(fields)...)) {
            fail();
        }
    }

    std::shared_ptr<R> object() { return m_object; }

    template <typename F, typename T>
    Self* field(F& field, T value) {
        field = value;
        return static_cast<Self*>(this);
    }

    void fail() { m_valid = false; }

private:
    std::shared_ptr<R> m_object;
    bool m_valid = true;
};

}  // namespace Util
