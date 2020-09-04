#include "bfio.h"
#include <memory>

bfio::bfio() noexcept
    : m_fp(nullptr), m_mode(0), m_format_buffer(0) {
}

bfio::bfio(const std::filesystem::__cxx11::path& path, bfio::OpenMode open_modes)
    : bfio() {
    open(path, open_modes);
}

void bfio::adopt(FILE* file, bfio::OpenMode mode) {
    m_fp = file;
    m_mode = mode;
    m_adopted = true;
}

bfio::~bfio() noexcept {
    if (!m_adopted) {
        if (std::fclose(m_fp) != 0) {
            // something went wrong, but we ignore it
        }
    }
}

void bfio::open(const std::filesystem::__cxx11::path& path, bfio::OpenMode open_modes) {
    m_path = path;
    m_mode = open_modes;
    std::string c_openmode {};
    if (m_mode & bfio::OpenMode::WRITE) {
        c_openmode += "w"; // write
    } else {
        c_openmode += "r"; // read
    }
    // due to how we read and write, we always open in binary mode.
    c_openmode += "b";
    m_fp = std::fopen(m_path.c_str(), c_openmode.c_str());
    if (m_fp == nullptr) {
        // posix requires that errno is set now, so we can use that
        throw std::runtime_error("Error trying to open \"" + m_path.string() + "\": " + std::string(std::strerror(errno)));
    }
}

void bfio::read_internal(bfio::Whitespace) {
    skip_all_of({ BFIO_WHITESPACE });
}

void bfio::read_internal(bfio::Ignore ignore) {
    for (size_t i = 0; i < ignore.count; ++i) {
        fgetc(m_fp);
    }
}

void bfio::read_internal(bfio::IgnoreLine) {
    char c;
    for (size_t i = 0; (c = fgetc(m_fp)) != EOF && c != '\n'; ++i) {
    }
    if (c != EOF) {
        fgetc(m_fp);
    }
}

void bfio::read_internal(bfio::u8& i) {
    std::string str;
    read_until_any_of(str, { BFIO_WHITESPACE });
    i = static_cast<u8>(std::stoul(str));
}

void bfio::read_internal(bfio::i8& i) {
    std::string str;
    read_until_any_of(str, { BFIO_WHITESPACE });
    i = static_cast<i8>(std::stoi(str));
}

void bfio::read_internal(bfio::u16& i) {
    std::string str;
    read_until_any_of(str, { BFIO_WHITESPACE });
    i = static_cast<u16>(std::stoul(str));
}

void bfio::read_internal(bfio::i16& i) {
    std::string str;
    read_until_any_of(str, { BFIO_WHITESPACE });
    i = static_cast<i16>(std::stoi(str));
}

void bfio::read_internal(bfio::u32& i) {
    std::string str;
    read_until_any_of(str, { BFIO_WHITESPACE });
    i = static_cast<u32>(std::stoul(str));
}

void bfio::read_internal(bfio::i32& i) {
    std::string str;
    read_until_any_of(str, { BFIO_WHITESPACE });
    i = static_cast<i32>(std::stoi(str));
}

void bfio::read_internal(bfio::u64& i) {
    std::string str;
    read_until_any_of(str, { BFIO_WHITESPACE });
    i = static_cast<u64>(std::stoul(str));
}

void bfio::read_internal(bfio::i64& i) {
    std::string str;
    read_until_any_of(str, { BFIO_WHITESPACE });
    i = static_cast<i64>(std::stol(str));
}

void bfio::read_internal(std::string& s) {
    s.clear();
    char c;
    while ((c = fgetc(m_fp)) != EOF && !std::isspace(c)) {
        s += c;
    }
    ungetc(c, m_fp);
}

size_t bfio::count_until_whitespace_or_eof() {
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

void bfio::write_raw(const byte* buffer, size_t size) {
    if (!(m_mode & bfio::WRITE)) {
        throw std::runtime_error("can't write to readonly file");
    }
    if (!buffer) {
        throw std::runtime_error("buffer is nullptr");
    }
    if (size == 0) {
        return;
    }
    auto ret = std::fwrite(buffer, sizeof(byte), size, m_fp);
    if (ret != size) {
        throw std::runtime_error("could not write all bytes (unknown fwrite error)");
    }
}

void bfio::read_raw(byte* buffer, size_t size) {
    if (m_mode & bfio::WRITE) {
        throw std::runtime_error("can't read from writeonly file");
    }
    auto ret = std::fread(buffer, sizeof(byte), size, m_fp);
    if (ret != size) {
        throw std::runtime_error("could not read desired size (unknown fwrite error)");
    }
}

void bfio::write_internal(const std::string& string) {
    auto [data, size] = as_byte_array<std::string::value_type>(string.c_str(), string.size());
    write_raw(data, size);
}

void bfio::write_internal(u8 uint8) {
    auto [data, size] = make_byte_buffer_fmt("%hhu", uint8);
    write_raw(data, size);
}

void bfio::write_internal(i8 int8) {
    auto [data, size] = make_byte_buffer_fmt("%hhi", int8);
    write_raw(data, size);
}

void bfio::write_internal(bfio::u16 i) {
    auto [data, size] = make_byte_buffer_fmt("%hu", i);
    write_raw(data, size);
}

void bfio::write_internal(bfio::i16 i) {
    auto [data, size] = make_byte_buffer_fmt("%hi", i);
    write_raw(data, size);
}

void bfio::write_internal(bfio::u32 i) {
    auto [data, size] = make_byte_buffer_fmt("%u", i);
    write_raw(data, size);
}

void bfio::write_internal(bfio::i32 i) {
    auto [data, size] = make_byte_buffer_fmt("%i", i);
    write_raw(data, size);
}

void bfio::write_internal(bfio::u64 i) {
    auto [data, size] = make_byte_buffer_fmt("%lu", i);
    write_raw(data, size);
}

void bfio::write_internal(bfio::i64 i) {
    auto [data, size] = make_byte_buffer_fmt("%li", i);
    write_raw(data, size);
}

void bfio::write_internal(void* ptr) {
    auto [data, size] = make_byte_buffer_fmt("%p", ptr);
    write_raw(data, size);
}

void bfio::write_char(char c, size_t count) {
    if (!(m_mode & bfio::WRITE)) {
        throw std::runtime_error("can't write to readonly file");
    }
    std::string printme;
    char tmp[2];
    tmp[0] = c;
    tmp[1] = '\0';
    for (size_t i = 0; i < count; ++i) {
        printme += tmp;
    }
    write_internal(printme);
}

void bfio::skip(size_t count) {
    if (m_mode & bfio::WRITE) {
        throw std::runtime_error("can't read from writeonly file");
    }
    read(bfio::Ignore(count));
}

void bfio::skip_whitespace() {
    if (m_mode & bfio::WRITE) {
        throw std::runtime_error("can't read from writeonly file");
    }
    char c;
    while (isspace(c = fgetc(m_fp)))
        ;
    ungetc(c, m_fp);
}

void bfio::skip_all_of(std::initializer_list<char> delims) {
    if (m_mode & bfio::WRITE) {
        throw std::runtime_error("can't read from writeonly file");
    }
    char c;
    while ((c = fgetc(m_fp)) != EOF && contains(delims, c)) {
    }
    ungetc(c, m_fp);
}

void bfio::skip_until(char delim) {
    if (m_mode & bfio::WRITE) {
        throw std::runtime_error("can't read from writeonly file");
    }
    char c;
    while ((c = fgetc(m_fp)) != EOF && c != delim) {
    }
    ungetc(c, m_fp);
}

void bfio::skip_line() {
    if (m_mode & bfio::WRITE) {
        throw std::runtime_error("can't read from writeonly file");
    }
    // like skip_until \n, but consumes the \n
    char c;
    while ((c = fgetc(m_fp)) != EOF && c != '\n') {
    }
}

void bfio::skip_until_any_of(std::initializer_list<char> delims) {
    if (m_mode & bfio::WRITE) {
        throw std::runtime_error("can't read from writeonly file");
    }
    char c;
    while ((c = fgetc(m_fp)) != EOF && !contains(delims, c)) {
    }
    ungetc(c, m_fp);
}

void bfio::read_until(std::string& s, char delim) {
    if (m_mode & bfio::WRITE) {
        throw std::runtime_error("can't read from writeonly file");
    }
    s.clear();
    char c;
    while ((c = fgetc(m_fp)) != EOF && c != delim) {
        s += c;
    }
    ungetc(c, m_fp);
}

void bfio::read_until_any_of(std::string& s, std::initializer_list<char> delims) {
    if (m_mode & bfio::WRITE) {
        throw std::runtime_error("can't read from writeonly file");
    }
    s.clear();
    char c;
    while ((c = fgetc(m_fp)) != EOF && !contains(delims, c)) {
        s += c;
    }
    ungetc(c, m_fp);
}

void bfio::read_line(std::string& s) {
    if (m_mode & bfio::WRITE) {
        throw std::runtime_error("can't read from writeonly file");
    }
    // just like read_until \n, but consumes \n.
    s.clear();
    char c;
    while ((c = fgetc(m_fp)) != EOF && c != '\n') {
        s += c;
    }
}

char bfio::peek() {
    if (m_mode & bfio::WRITE) {
        throw std::runtime_error("can't read from writeonly file");
    }
    char c = fgetc(m_fp);
    ungetc(c, m_fp);
    return c;
}

bool bfio::reached_eof() {
    // TODO: Maybe throw if writeonly
    return feof(m_fp) != 0;
}

bfio::Ignore::Ignore(size_t _count)
    : count(_count) {
}

bfio::Whitespace::Whitespace(size_t _count)
    : count(_count) { }
