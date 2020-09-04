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
        explicit Whitespace(size_t _count = 1)
            : count(_count) { }
    };

    struct Ignore {
        size_t count;
        explicit Ignore(size_t _count = 1)
            : count(_count) {
        }
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
    inline std::pair<const byte*, size_t> make_byte_buffer_fmt(const char* format_string, Args&&... args) {
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

    struct EndRead {
    };

    template<typename... Args>
    void read_internal(EndRead) {
        // end :)
    }

    template<typename... Args>
    void read_internal(Whitespace space, Args&&... args) {
        throw std::runtime_error("not implemented");
    }

    template<typename... Args>
    void read_internal(Ignore ignore, Args&&... args) {
        for (size_t i = 0; i < ignore.count; ++i) {
            fgetc(m_fp);
        }
        read_internal(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void read_internal(IgnoreLine, Args&&... args) {
        char c;
        for (size_t i = 0; (c = fgetc(m_fp)) != EOF && c != '\n'; ++i) {
        }
        if (c != EOF) {
            fgetc(m_fp);
        }
        read_internal(std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void read_internal(char& i, Args&&... args) {
        auto ret = fscanf(m_fp, "%c", &i);
        if (ret == EOF) {
            throw std::runtime_error("couldn't parse value in fscanf");
        }
        read_internal(std::forward<Args>(args)...);
    }
    
    template<typename... Args>
    void read_internal(u8& i, Args&&... args) {
        auto ret = fscanf(m_fp, "%hhu", &i);
        if (ret == EOF) {
            throw std::runtime_error("couldn't parse value in fscanf");
        }
        read_internal(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void read_internal(i8& i, Args&&... args) {
        auto ret = fscanf(m_fp, "%hhi", &i);
        if (ret == EOF) {
            throw std::runtime_error("couldn't parse value in fscanf");
        }
        read_internal(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void read_internal(u16& i, Args&&... args) {
        auto ret = fscanf(m_fp, "%hu", &i);
        if (ret == EOF) {
            throw std::runtime_error("couldn't parse value in fscanf");
        }
        read_internal(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void read_internal(i16& i, Args&&... args) {
        auto ret = fscanf(m_fp, "%hi", &i);
        if (ret == EOF) {
            throw std::runtime_error("couldn't parse value in fscanf");
        }
        read_internal(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void read_internal(u32& i, Args&&... args) {
        auto ret = fscanf(m_fp, "%u", &i);
        if (ret == EOF) {
            throw std::runtime_error("couldn't parse value in fscanf");
        }
        read_internal(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void read_internal(i32& i, Args&&... args) {
        auto ret = fscanf(m_fp, "%i", &i);
        if (ret == EOF) {
            throw std::runtime_error("couldn't parse value in fscanf");
        }
        read_internal(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void read_internal(u64& i, Args&&... args) {
        auto ret = fscanf(m_fp, "%lu", &i);
        if (ret == EOF) {
            throw std::runtime_error("couldn't parse value in fscanf");
        }
        read_internal(std::forward<Args>(args)...);
    }

    template<typename... Args>
    void read_internal(i64& i, Args&&... args) {
        auto ret = fscanf(m_fp, "%li", &i);
        if (ret == EOF) {
            throw std::runtime_error("couldn't parse value in fscanf");
        }
        read_internal(std::forward<Args>(args)...);
    }

    size_t count_until_whitespace_or_eof() {
        size_t count;
        std::vector<char> chars;
        chars.reserve(40);
        chars.emplace_back(fgetc(m_fp));
        for (count = 1 /* 1? */; !std::isspace(chars.back()); ++count) {
            char tmp = fgetc(m_fp);
            std::cout << "_" << tmp << "_ is not space ..." << std::endl;
            if (tmp == EOF)
                break;
            else
                chars.emplace_back(tmp);
        }
        std::reverse(chars.begin(), chars.end());
        for (char& put_c : chars) {
            std::cout << "putting back _" << put_c << "_" << std::endl;
            ungetc(put_c, m_fp);
        }
        std::cout << "size: " << count << std::endl;
        return count;
    }

    template<typename... Args>
    void read_internal(std::string& s, Args&&... args) {
        s.clear();
        char c;
        while ((c = fgetc(m_fp)) != EOF && !std::isspace(c)) {
            s += c;
        }
        ungetc(c, m_fp);
        read_internal(std::forward<Args>(args)...);
    }

    template<typename STLContainerT, typename ValueT = decltype(STLContainerT::value_type)>
    static bool contains(const STLContainerT& container, ValueT value) {
        return std::find(container.begin(), container.end(), value) != container.end();
    }

public:
    // public interface

    template<typename... Args>
    void read(Args&&... args) {
        read_internal(std::forward<Args>(args)..., EndRead());
    }

    void skip(size_t count) {
        read(bfio::Ignore(count));
    }

    void skip_whitespace() {
        char c;
        while (isspace(c = fgetc(m_fp)))
            ;
        ungetc(c, m_fp);
    }

    void skip_until(char delim) {
        char c;
        while ((c = fgetc(m_fp)) != EOF && c != delim) {
        }
        ungetc(c, m_fp);
    }

    void skip_line() {
        // like skip_until \n, but consumes the \n
        char c;
        while ((c = fgetc(m_fp)) != EOF && c != '\n') {
        }
    }

    void skip_until_any_of(std::initializer_list<char> delims) {
        char c;
        while ((c = fgetc(m_fp)) != EOF && !contains(delims, c)) {
        }
        ungetc(c, m_fp);
    }

    void read_until(std::string& s, char delim) {
        s.clear();
        char c;
        while ((c = fgetc(m_fp)) != EOF && c != delim) {
            s += c;
        }
        ungetc(c, m_fp);
    }

    void read_until_any_of(std::string& s, std::initializer_list<char> delims) {
        s.clear();
        char c;
        while ((c = fgetc(m_fp)) != EOF && !contains(delims, c)) {
            s += c;
        }
        ungetc(c, m_fp);
    }

    void read_line(std::string& s) {
        // just like read_until \n, but consumes \n.
        s.clear();
        char c;
        while ((c = fgetc(m_fp)) != EOF && c != '\n') {
            s += c;
        }
    }

    bool reached_eof() {
        return feof(m_fp) != 0;
    }

    /// initialize, dont open anything
    bfio() noexcept;
    /// open from filesytem::path. OR together openmodes.
    bfio(const std::filesystem::path& path, OpenMode open_modes);

    void adopt(FILE* file, OpenMode mode);

    ~bfio() noexcept;

    void open(const std::filesystem::path& path, OpenMode open_modes);

    void write(const byte* buffer, size_t size);
    void write(const std::string& string);
    void write(u8 uint8);
    void write(i8 int8);
    void write(u16 i);
    void write(i16 i);
    void write(u32 i);
    void write(i32 i);
    void write(u64 i);
    void write(i64 i);
    void write(void* ptr);
    void write_char(char c, size_t count = 1);
};


#endif // BFIO_H
