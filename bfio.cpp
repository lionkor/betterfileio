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

void bfio::write(const byte* buffer, size_t size) {
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

void bfio::write(const std::string& string) {
    auto [data, size] = as_byte_array<std::string::value_type>(string.c_str(), string.size());
    write(data, size);
}

void bfio::write(u8 uint8) {
    auto [data, size] = make_byte_buffer_fmt("%hhu", uint8);
    write(data, size);
}

void bfio::write(i8 int8) {
    auto [data, size] = make_byte_buffer_fmt("%hhi", int8);
    write(data, size);
}

void bfio::write(bfio::u16 i) {
    auto [data, size] = make_byte_buffer_fmt("%hu", i);
    write(data, size);
}

void bfio::write(bfio::i16 i) {
    auto [data, size] = make_byte_buffer_fmt("%hi", i);
    write(data, size);
}

void bfio::write(bfio::u32 i) {
    auto [data, size] = make_byte_buffer_fmt("%u", i);
    write(data, size);
}

void bfio::write(bfio::i32 i) {
    auto [data, size] = make_byte_buffer_fmt("%i", i);
    write(data, size);
}

void bfio::write(bfio::u64 i) {
    auto [data, size] = make_byte_buffer_fmt("%lu", i);
    write(data, size);
}

void bfio::write(bfio::i64 i) {
    auto [data, size] = make_byte_buffer_fmt("%li", i);
    write(data, size);
}

void bfio::write(void* ptr) {
    auto [data, size] = make_byte_buffer_fmt("%p", ptr);
    write(data, size);
}

void bfio::write_char(char c, size_t count) {
    std::string printme;
    char tmp[2];
    tmp[0] = c;
    tmp[1] = '\0';
    for (size_t i = 0; i < count; ++i) {
        printme += tmp;
    }
    write(printme);
}
