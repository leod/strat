#ifndef STRAT_COMMON_BITSTREAM_HH
#define STRAT_COMMON_BITSTREAM_HH

#include <cstdint>
#include <vector>
#include <string>
#include <type_traits>
#include <iostream>
#include <typeinfo>

// These bit streams operate on a byte level... for now. :P
// Doesn't take care of endianness issues.

struct BitStreamWriter {
    BitStreamWriter();

    void writeBytes(uint8_t const* data, size_t size);

    uint8_t const* ptr() const;
    size_t size() const;

    void reset();

private:
    std::vector<uint8_t> buffer;
};

struct BitStreamReader {
    BitStreamReader(std::vector<uint8_t> const&);
    BitStreamReader(uint8_t const* buffer, size_t bufferLength);

    void readBytes(uint8_t* out, size_t size);

    bool eof();

    size_t position() const { return index; }

    std::vector<uint8_t> restVector() const;

    void skip(size_t offset);

private:
    uint8_t const* buffer;
    size_t const bufferLength;

    size_t index;
};

template<typename T/*,
         typename = typename std::enable_if<std::is_pod<T>::value>::type*/>
void write(BitStreamWriter& stream, const T& value) {
    stream.writeBytes(reinterpret_cast<uint8_t const*>(&value), sizeof(T));
}

void write(BitStreamWriter&, std::string const&);

template<typename T>
void write(BitStreamWriter& stream, const std::vector<T>& v) {
    write(stream, v.size());

    for (auto& e : v)
        write(stream, e);
}

template<typename T/*,
         typename = typename std::enable_if<std::is_pod<T>::value>::type*/>
void read(BitStreamReader& stream, T& value) {
    stream.readBytes(reinterpret_cast<uint8_t*>(&value), sizeof(T));
}

void read(BitStreamReader&, std::string&);

template<typename T>
void read(BitStreamReader& stream, std::vector<T>& v) {
    size_t size;
    read(stream, size);

    v.resize(size);

    for (auto& e : v)
        read(stream, e);
}

#endif
