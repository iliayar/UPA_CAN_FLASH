#pragma once

#include "bytes.h"

namespace Util {

template <typename T, int size>
struct Field {
    Field(T value) : m_value(value), m_valid(true) {}
    Field() : m_value(), m_valid(false) {}
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

    Field<T, size>& operator=(const T value) {
        m_value = value;
        m_valid = true;
        return *this;
    }

    void set(T value) {
        m_value = value;
        m_valid = true;
    }

    int get_size() {
        return size;
    }

    bool valid() { return m_valid; }

protected:
    virtual optional<T> read_impl(Reader& read) = 0;
    virtual bool write_impl(Writer& read, T value) = 0;

private:
    T m_value;
    bool m_valid = false;
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

template <int size>
struct VecField : public Field<std::vector<uint8_t>, size> {
    VecField(std::vector<uint8_t> data)
        : Field<std::vector<uint8_t>, size>(data) {}
    VecField() : Field<std::vector<uint8_t>, size>() {}

protected:
    bool write_impl(Writer& writer, std::vector<uint8_t> value) {
        return writer.write(value, size);
    }
    optional<std::vector<uint8_t>> read_impl(Reader& reader) {
        return reader.read(size);
    }
};

template <typename T, typename I, int size>
struct EnumField : public Field<T, size> {
    EnumField(T value) : Field<T, size>(value) {}
    EnumField() : Field<T, size>() {}

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

template <typename ...F>
void write_args(Writer& writer, F&... args) {
    over_all<F&...>::for_each([&](auto field) {
        field.write(writer);
    }, std::forward<F&>(args)...);
}

template <typename... F>
std::vector<uint8_t> dump_args(F&... args) {
    int size = 0;
    over_all<F&...>::for_each([&](auto field) { size += field.get_size(); },
                             std::forward<F&>(args)...);
    Writer writer(BYTES(size));
    write_args(writer, std::forward<F&>(args)...);
    return writer.get_payload();
}

template <typename... F>
bool read_args(Reader& reader, F&... args) {
    bool res = true;
    over_all<F&...>::for_each(
        [&](auto field) {
            field.read(reader);
            res &= field.valid();
        },
        std::forward<F&>(args)...);
    return res;
}

template <typename R>
class Builder {
public:
    Builder() { m_object = std::shared_ptr<R>(); }

    optional<std::shared_ptr<R>> build() {
        if (!m_valid) {
            return {};
        }
        return m_object;
    }

protected:

    using Self = Builder<R>;

    template <typename... F>
    void read(Reader& reader, F&... fields) {
        if (!read_args(reader, std::forward<F&>(fields)...)) {
            m_valid = false;
        }
    }

    std::shared_ptr<R> object() {
        return m_object;
    }

    std::shared_ptr<Self> self() {
        return std::unique_ptr<Self>(this);
    }

    template <typename F, typename T>
    std::shared_ptr<Self> field(F& field, T value) {
        field = value;
        return self();
    }

private:
    std::shared_ptr<R> m_object;
    bool m_valid = true;
};

}
