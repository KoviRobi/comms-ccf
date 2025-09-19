#include "cbor.hpp"

#if defined(DEBUG_CBOR)
#include DEBUG_CBOR
#else
#include "ndebug.hpp"
#endif

#include <limits.h>
#include <stdint.h>

#include <concepts>
#include <optional>
#include <span>

bool Cbor::packEmbedded(Major major, uint8_t value, std::span<uint8_t> & buf)
{
    if (buf.size() >= 1 && value <= EMBEDDED_MAX)
    {
        buf[0] = setMajor(value, major);
        debugf(DEBUG "pack embedded %02X" END LOGLEVEL_ARGS, buf[0]);
        buf = buf.subspan(1);
        return true;
    }
    return false;
}

namespace
{
    template<std::unsigned_integral Int>
    bool pack(Cbor::Major major, Int value, std::span<uint8_t> & buf)
    {
        if (buf.size() < 1 + sizeof(Int))
        {
            return false;
        }
        switch (sizeof(Int))
        {
            case 1:
            {
                buf[0] = initialByte(major, Cbor::Minor::OneByteFollows);
                buf[1] = static_cast<uint8_t>(value);
                debugf(DEBUG "pack 8b %02X:%02X" END LOGLEVEL_ARGS,
                    buf[0], buf[1]);
                buf = buf.subspan(2);
                return true;
            }

            case 2:
            {
                buf[0] = initialByte(major, Cbor::Minor::TwoByteFollows);
                buf[1] = static_cast<uint8_t>(value >> 8);
                buf[2] = static_cast<uint8_t>(value);
                debugf(DEBUG "pack 16b %02X:%02X%02X" END LOGLEVEL_ARGS,
                    buf[0], buf[1], buf[2]);
                buf = buf.subspan(3);
                return true;
            }

            case 4:
            {
                buf[0] = initialByte(major, Cbor::Minor::FourByteFollows);
                buf[1] = static_cast<uint8_t>(value >> 24);
                buf[2] = static_cast<uint8_t>(value >> 16);
                buf[3] = static_cast<uint8_t>(value >> 8);
                buf[4] = static_cast<uint8_t>(value);
                debugf(DEBUG "pack 32b %02X:%02X%02X %02X%02X" END LOGLEVEL_ARGS,
                    buf[0], buf[1], buf[2], buf[3], buf[4]);
                buf = buf.subspan(5);
                return true;
            }

            case 8:
            {
                buf[0] = initialByte(major, Cbor::Minor::EightByteFollows);
                buf[1] = static_cast<uint8_t>(static_cast<uint64_t>(value) >> 56);
                buf[2] = static_cast<uint8_t>(static_cast<uint64_t>(value) >> 48);
                buf[3] = static_cast<uint8_t>(static_cast<uint64_t>(value) >> 40);
                buf[4] = static_cast<uint8_t>(static_cast<uint64_t>(value) >> 32);
                buf[5] = static_cast<uint8_t>(static_cast<uint64_t>(value) >> 24);
                buf[6] = static_cast<uint8_t>(static_cast<uint64_t>(value) >> 16);
                buf[7] = static_cast<uint8_t>(static_cast<uint64_t>(value) >> 8);
                buf[8] = static_cast<uint8_t>(static_cast<uint64_t>(value));
                debugf(DEBUG "pack 64b %02X:%02X%02X %02X%02X %02X%02X %02X%02X" END LOGLEVEL_ARGS,
                    buf[0], buf[1], buf[2], buf[3], buf[4], buf[5], buf[6], buf[7], buf[8]);
                buf = buf.subspan(9);
                return true;
            }

            default: break;
        }
        return false;
    }
}
template<>
bool Cbor::pack<unsigned char>(Major major, unsigned char value, std::span<uint8_t> & buf)
{
    return ::pack(major, value, buf);
}
template<>
bool Cbor::pack<unsigned short>(Major major, unsigned short value, std::span<uint8_t> & buf)
{
    return ::pack(major, value, buf);
}
template<>
bool Cbor::pack<unsigned int>(Major major, unsigned int value, std::span<uint8_t> & buf)
{
    return ::pack(major, value, buf);
}
template<>
bool Cbor::pack<unsigned long>(Major major, unsigned long value, std::span<uint8_t> & buf)
{
    return ::pack(major, value, buf);
}
template<>
bool Cbor::pack<unsigned long long>(Major major, unsigned long long value, std::span<uint8_t> & buf)
{
    return ::pack(major, value, buf);
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
            debugf(DEBUG "unpack embedded %02X->%02X" END LOGLEVEL_ARGS, initialByte, value);
            buf = buf.subspan(1);
            return  {{major, minor, value}};
        }
        else if (minor == Minor::OneByteFollows && buf.size() >= 2)
        {
            uint8_t value = buf[1];
            debugf(DEBUG "unpack 8b %02X" END LOGLEVEL_ARGS, value);
            buf = buf.subspan(2);
            return {{major, minor, value}};
        }
        else if (minor == Minor::TwoByteFollows && buf.size() >= 3)
        {
            uint16_t value = (static_cast<uint16_t>(buf[1]) << 8) | (static_cast<uint16_t>(buf[2]));
            debugf(DEBUG "unpack 16b %04X" END LOGLEVEL_ARGS, value);
            buf = buf.subspan(3);
            return {{major, minor, value}};
        }
        else if (minor == Minor::FourByteFollows && buf.size() >= 5)
        {
            uint32_t value =
                (static_cast<uint32_t>(buf[1]) << 24) | (static_cast<uint32_t>(buf[2]) << 16) |
                (static_cast<uint32_t>(buf[3]) <<  8) | (static_cast<uint32_t>(buf[4]));
            debugf(DEBUG "unpack 32b %08X" END LOGLEVEL_ARGS, value);
            buf = buf.subspan(5);
            return {{major, minor, value}};
        }
        else if (minor == Minor::EightByteFollows && buf.size() >= 9)
        {
            uint64_t value =
                (static_cast<uint64_t>(buf[1]) << 56) | (static_cast<uint64_t>(buf[2]) << 48) |
                (static_cast<uint64_t>(buf[3]) << 40) | (static_cast<uint64_t>(buf[4]) << 32) |
                (static_cast<uint64_t>(buf[5]) << 24) | (static_cast<uint64_t>(buf[6]) << 16) |
                (static_cast<uint64_t>(buf[7]) <<  8) | (static_cast<uint64_t>(buf[8]));
            debugf(DEBUG "unpack 64b %08X%08X" END LOGLEVEL_ARGS, (int)(value>>32), (int)(value));
            buf = buf.subspan(9);
            return {{major, minor, value}};
        }
    }
    return {};
}

template<>
bool Cbor::encode<unsigned char>(Major major, unsigned char value, std::span<uint8_t> & buf)
{
    return value > EMBEDDED_MAX
        ? pack<unsigned char>(major, value, buf)
        : packEmbedded(major, static_cast<uint8_t>(value), buf);
}

template<>
bool Cbor::encode<unsigned short>(Major major, unsigned short value, std::span<uint8_t> & buf)
{
    return value > USHRT_MAX
        ? pack<unsigned short>(major, value, buf)
        : encode<unsigned char>(major, static_cast<unsigned char>(value), buf);
}

template<>
bool Cbor::encode<unsigned int>(Major major, unsigned int value, std::span<uint8_t> & buf)
{
    return value > UINT16_MAX
        ? pack<unsigned int>(major, value, buf)
        : encode<uint16_t>(major, static_cast<uint16_t>(value), buf);
}

template<>
bool Cbor::encode<unsigned long>(Major major, unsigned long value, std::span<uint8_t> & buf)
{
    return value > UINT_MAX
        ? pack<unsigned long>(major, value, buf)
        : encode<unsigned>(major, static_cast<unsigned>(value), buf);
}

template<>
bool Cbor::encode<unsigned long long>(Major major, unsigned long long value, std::span<uint8_t> & buf)
{
    return value > ULONG_MAX
        ? pack<unsigned long long>(major, value, buf)
        : encode<unsigned long>(major, static_cast<unsigned long>(value), buf);
}
