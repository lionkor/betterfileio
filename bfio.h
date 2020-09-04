#ifndef BFIO_H
#define BFIO_H

#include <string>
#include <filesystem>
#include <cstring>
#include <iostream>
#include <cstdio>
#include <vector>
#include <algorithm>
#include "bitops/bitops.h"

#define BFIO_WHITESPACE '\t', ' ', '\n', '\r'
#define fscanf FUCK
#define scanf FUCK

class bfio final
{
public:
    // types
    using char_type = char;
    using u8 = uint8_t;
    using i8 = int8_t;
    using u16 = uint16_t;
    using i16 = int16_t;
    using u32 = uint32_t;
    using i32 = int32_t;
    using u64 = uint64_t;
    using i64 = int64_t;
    using byte = std::byte;

    enum OpenMode : u8
    {
        // read and write are exclusive, its either one or the other.
        // therefore they set / unset the same bit (the "write" bit)
        READ = 0, // can't check against this
        WRITE = 1
    };

    struct Whitespace {
        size_t count;
        explicit Whitespace(size_t _count = 1);
    };


    struct Ignore {
        size_t count;
        explicit Ignore(size_t _count = 1);
    };

    struct IgnoreLine {
    };

private:
    // fields & private methods
    std::FILE* m_fp { nullptr };
    u8 m_mode;
    std::filesystem::path m_path;
    // buffer used for format strings so we dont need to alloca or
    // new[] every time we need a resizable buffer (that might be similar size
    // every time
    std::vector<char> m_format_buffer;
    bool m_adopted { false }; // we dont fclose adopted streams, like stdout

    template<typename T>
    static inline std::pair<const byte*, size_t> as_byte_array(const T* obj, size_t count) {
        return { reinterpret_cast<const bfio::byte*>(obj), count * sizeof(T) };
    }

    /// ATTENTION: the resulting pointer is invalid when you call the function agaín
    template<typename... Args>
    std::pair<const byte*, size_t> make_byte_buffer_fmt(const char* format_string, Args&&... args);

    struct EndRead {
    };

    size_t count_until_whitespace_or_eof();

    void read_internal(Whitespace);
    void read_internal(Ignore ignore);
    void read_internal(IgnoreLine);
    void read_internal(u8& i);
    void read_internal(i8& i);
    void read_internal(u16& i);
    void read_internal(i16& i);
    void read_internal(u32& i);
    void read_internal(i32& i);
    void read_internal(u64& i);
    void read_internal(i64& i);
    void read_internal(std::string& s);

    void write_raw(const byte* buffer, size_t size);
    void write_internal(const std::string& string);
    void write_internal(u8 uint8);
    void write_internal(i8 int8);
    void write_internal(u16 i);
    void write_internal(i16 i);
    void write_internal(u32 i);
    void write_internal(i32 i);
    void write_internal(u64 i);
    void write_internal(i64 i);
    void write_internal(void* ptr);


    template<typename STLContainerT, typename ValueT = decltype(STLContainerT::value_type)>
    static bool contains(const STLContainerT& container, ValueT value) {
        return std::find(container.begin(), container.end(), value) != container.end();
    }


    template<class T, typename... Args>
    void read_start(T& obj, Args&&... args) {
        read_internal(obj);
        read_start(std::forward<Args>(args)...);
    }

    template<class T, typename... Args>
    void read_start(T&& obj, Args&&... args) {
        read_internal(std::forward<T>(obj));
        read_start(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void read_start(EndRead) {
        // done :)
    }

    template<class T, typename... Args>
    void write_start(T obj, Args&&... args) {
        write_internal(obj);
        write_start(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void write_start(EndRead) {
        // done :)
    }

public:
    // public interface

    void write_char(char c, size_t count = 1);

    template<typename... Args>
    void read(Args&&... args) {
        try {
            read_start(std::forward<Args>(args)..., EndRead());
        } catch (const std::invalid_argument& e) {
            throw std::runtime_error(std::string("read/conversion failed due to invalid argument: ") + e.what());
        } catch (const std::runtime_error& e) {
            throw;
        }
    }

    template<typename... Args>
    void write(Args&&... args) {
        write_start(std::forward<Args>(args)..., EndRead());
    }

    void skip(size_t count = 1);

    void skip_whitespace() {
        char c;
        while (isspace(c = fgetc(m_fp)))
            ;
        ungetc(c, m_fp);
    }

    void skip_all_of(std::initializer_list<char> delims);
    void skip_until(char delim);
    void skip_line();
    void skip_until_any_of(std::initializer_list<char> delims);
    void read_until(std::string& s, char delim);
    void read_until_any_of(std::string& s, std::initializer_list<char> delims);
    void read_line(std::string& s);

    bool reached_eof();

    /// initialize, dont open anything
    bfio() noexcept;
    /// open from filesytem::path. OR together openmodes.
    bfio(const std::filesystem::path& path, OpenMode open_modes);
    ~bfio() noexcept;

    void adopt(FILE* file, OpenMode mode);
    void open(const std::filesystem::path& path, OpenMode open_modes);
};

template<typename... Args>
std::pair<const bfio::byte*, size_t> bfio::make_byte_buffer_fmt(const char* format_string, Args&&... args) {
    // figure out size
    auto size = std::snprintf(nullptr, 0, format_string, std::forward<Args>(args)...);
    // we use a class-scope buffer here so we don't heap alloc necessarily every time we
    // format.
    if (m_format_buffer.size() < size_t(size + 1)) {
        m_format_buffer.resize(size + 1);
    }
    std::fill(m_format_buffer.begin(), m_format_buffer.end(), '\0');
    auto count = std::sprintf(m_format_buffer.data(), format_string, std::forward<Args>(args)...);
    if (count != size) {
        throw std::runtime_error("something went wrong writing format string");
    }
    return { reinterpret_cast<const byte*>(m_format_buffer.data()), count * sizeof(char) };
}


#endif // BFIO_H
