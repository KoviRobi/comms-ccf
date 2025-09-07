#include "cbor.hpp"

#if defined(DEBUG_CBOR)
#include DEBUG_CBOR
#else
extern int printf(const char *, ...);
/// Need an expression that contains parameters for parameter pack
/// Want to avoid linking printf at any optimisation, hence the constexpr if
/// Want an expression not a statement, hence the lambda wrapper
#define debugf(...) ([&]{ if constexpr (false) { printf(__VA_ARGS__); } }())
#endif

#include <stdint.h>

#include <span>
#include <optional>

bool Cbor::packEmbedded(Major major, uint8_t value, std::span<uint8_t> & buf)
{
    if (buf.size() >= 1 && value <= EMBEDDED_MAX)
    {
        buf[0] = setMajor(value, major);
        debugf("pack embedded %02X\n", buf[0]);
        buf = buf.subspan(1);
        return true;
    }
    return false;
}

template<>
bool Cbor::pack<uint8_t, 1>(Major major, uint8_t value, std::span<uint8_t> & buf)
{
    if (buf.size() >= 2)
    {
        buf[0] = initialByte(major, Minor::OneByteFollows);
        buf[1] = static_cast<uint8_t>(value);
        debugf("pack 8b %02X:%02X\n", buf[0], buf[1]);
        buf = buf.subspan(2);
        return true;
    }
    return false;
}

template<>
bool Cbor::pack<uint16_t, 2>(Major major, uint16_t value, std::span<uint8_t> & buf)
{
    if (buf.size() >= 3)
    {
        buf[0] = initialByte(major, Minor::TwoByteFollows);
        buf[1] = static_cast<uint8_t>(value >> 8);
        buf[2] = static_cast<uint8_t>(value);
        debugf("pack 16b %02X:%02X%02X\n", buf[0], buf[1], buf[2]);
        buf = buf.subspan(3);
        return true;
    }
    return false;
}

template<>
bool Cbor::pack<uint32_t, 4>(Major major, uint32_t value, std::span<uint8_t> & buf)
{
    if (buf.size() >= 5)
    {
        buf[0] = initialByte(major, Minor::FourByteFollows);
        buf[1] = static_cast<uint8_t>(value >> 24);
        buf[2] = static_cast<uint8_t>(value >> 16);
        buf[3] = static_cast<uint8_t>(value >> 8);
        buf[4] = static_cast<uint8_t>(value);
        debugf("pack 32b %02X:%02X%02X %02X%02X\n", buf[0], buf[1], buf[2], buf[3], buf[4]);
        buf = buf.subspan(5);
        return true;
    }
    return false;
}

template<>
bool Cbor::pack<uint64_t, 8>(Major major, uint64_t value, std::span<uint8_t> & buf)
{
    if (buf.size() >= 9)
    {
        buf[0] = initialByte(major, Minor::EightByteFollows);
        buf[1] = static_cast<uint8_t>(value >> 56);
        buf[2] = static_cast<uint8_t>(value >> 48);
        buf[3] = static_cast<uint8_t>(value >> 40);
        buf[4] = static_cast<uint8_t>(value >> 32);
        buf[5] = static_cast<uint8_t>(value >> 24);
        buf[6] = static_cast<uint8_t>(value >> 16);
        buf[7] = static_cast<uint8_t>(value >> 8);
        buf[8] = static_cast<uint8_t>(value);
        debugf("pack 32b %02X:%02X%02X %02X%02X %02X%02X %02X%02X\n", buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8]);
        buf = buf.subspan(9);
        return true;
    }
    return false;
}

std::optional<Cbor::Item> Cbor::unpack(std::span<uint8_t> & buf)
{
    if (buf.size() >= 1)
    {
        uint8_t initialByte = buf[0];
        Major major = getMajor(initialByte);
        Minor minor = getMinor(initialByte);
        if (minor <= Minor::EmbeddedLast || minor == Minor::Indefinite)
        {
            uint8_t value = static_cast<uint8_t>(minor);
            debugf("unpack embedded %02X->%02X\n", initialByte, value);
            buf = buf.subspan(1);
            return  {{major, minor, value}};
        }
        else if (minor == Minor::OneByteFollows && buf.size() >= 2)
        {
            uint8_t value = buf[1];
            debugf("unpack 8b %02X\n", value);
            buf = buf.subspan(2);
            return {{major, minor, value}};
        }
        else if (minor == Minor::TwoByteFollows && buf.size() >= 3)
        {
            uint16_t value = (static_cast<uint16_t>(buf[1]) << 8) | (static_cast<uint16_t>(buf[2]));
            debugf("unpack 16b %04X\n", value);
            buf = buf.subspan(3);
            return {{major, minor, value}};
        }
        else if (minor == Minor::FourByteFollows && buf.size() >= 5)
        {
            uint32_t value =
                (static_cast<uint32_t>(buf[1]) << 24) | (static_cast<uint32_t>(buf[2]) << 16) |
                (static_cast<uint32_t>(buf[3]) <<  8) | (static_cast<uint32_t>(buf[4]));
            debugf("unpack 32b %08X\n", value);
            buf = buf.subspan(5);
            return {{major, minor, value}};
        }
        else if (minor == Minor::EightByteFollows && buf.size() >= 9)
        {
            debugf("unpack 64b\n");
            uint64_t value =
                (static_cast<uint64_t>(buf[1]) << 56) | (static_cast<uint64_t>(buf[2]) << 48) |
                (static_cast<uint64_t>(buf[3]) << 40) | (static_cast<uint64_t>(buf[4]) << 32) |
                (static_cast<uint64_t>(buf[5]) << 24) | (static_cast<uint64_t>(buf[6]) << 16) |
                (static_cast<uint64_t>(buf[7]) <<  8) | (static_cast<uint64_t>(buf[8]));
            debugf("unpack 32b %08X%08X\n", (int)(value>>32), (int)(value));
            buf = buf.subspan(9);
            return {{major, minor, value}};
        }
    }
    return {};
}

template<>
bool Cbor::encode<uint8_t, 1>(Major major, uint8_t value, std::span<uint8_t> & buf)
{
    return value > EMBEDDED_MAX
        ? pack<uint8_t, 1>(major, static_cast<uint8_t>(value), buf)
        : packEmbedded(major, static_cast<uint8_t>(value), buf);
}

template<>
bool Cbor::encode<uint16_t, 2>(Major major, uint16_t value, std::span<uint8_t> & buf)
{
    return value > UINT8_MAX
        ? pack<uint16_t, 2>(major, static_cast<uint16_t>(value), buf)
        : encode<uint8_t, 1>(major, static_cast<uint8_t>(value), buf);
}

template<>
bool Cbor::encode<uint32_t, 4>(Major major, uint32_t value, std::span<uint8_t> & buf)
{
    return value > UINT16_MAX
        ? pack<uint32_t, 4>(major, static_cast<uint32_t>(value), buf)
        : encode<uint16_t, 2>(major, static_cast<uint16_t>(value), buf);
}

template<>
bool Cbor::encode<uint64_t, 8>(Major major, uint64_t value, std::span<uint8_t> & buf)
{
    return value > UINT32_MAX
        ? pack<uint64_t, 8>(major, value, buf)
        : encode<uint32_t, 4>(major, static_cast<uint32_t>(value), buf);
}
