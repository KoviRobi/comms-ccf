/**

# FNV-1A hash

To verify message integrity, you need some kind of a checksum or hash. I have
chosen the FNV-1A hash because it is extremely simple to implement in software
(only marginally more complicated than the "add every byte" hash sometimes
known as lose-lose because of its terrible performance).

There are faster hashes (they tend to be faster because they work more than a
byte at a time) but when the input is slowly coming over UART this works
perfectly fine.

This isn't meant to be a cryptographic hash (FNV-1A was never intended to be
that). So it isn't used for signing, only to detect errors.

For a simple comparison of other hashes, see
test/hash_comparison.py but for a much better one see
https://softwareengineering.stackexchange.com/a/145633

*/

#pragma once

#if defined(DEBUG_FNV1A)
#include DEBUG_FNV1A
#else
#include "ndebug.hpp"
#endif

#include <stddef.h>
#include <stdint.h>

#include <span>

namespace Fnv1a
{
    constexpr uint32_t initialHash = 0x811c9dc5;

    constexpr uint32_t feed(uint32_t hash, uint8_t byte)
    {
        hash ^= byte;
        hash *= 0x01000193;
        return hash;
    }

    template<size_t Size>
    constexpr uint32_t checksum(std::span<uint8_t, Size> span, uint32_t hash = initialHash)
    {
        for (auto c : span)
        {
            hash = feed(hash, c);
        }
        return hash;
    }

    template<size_t Size>
    constexpr void putAtEnd(std::span<uint8_t, Size> span, uint32_t hash = initialHash)
    {
        size_t end = span.size();
        if (end < 4)
        {
            return;
        }
        const uint32_t got = checksum(span.first(end - 4), hash);
        span[end - 4] = static_cast<uint32_t>(got) >>  0;
        span[end - 3] = static_cast<uint32_t>(got) >>  8;
        span[end - 2] = static_cast<uint32_t>(got) >> 16;
        span[end - 1] = static_cast<uint32_t>(got) >> 24;
    }

    template<size_t Size>
    constexpr bool checkAtEnd(std::span<uint8_t, Size> span, uint32_t hash = initialHash)
    {
        size_t end = span.size();
        if (end < 4)
        {
            return false;
        }
        const uint32_t got = checksum(span.first(end - 4), hash);
        const uint32_t expected =
                (static_cast<uint32_t>(span[end - 4]) <<  0) |
                (static_cast<uint32_t>(span[end - 3]) <<  8) |
                (static_cast<uint32_t>(span[end - 2]) << 16) |
                (static_cast<uint32_t>(span[end - 1]) << 24);
        if (got == expected)
        {
            debugf(INFO "Checksum OK" END, START);
        }
        else
        {
            debugf(INFO "Checksum got %08X expected %08X" END, START, got, expected);
        }
        return got == expected;
    }
};

// This is a header, undefine the debugf macro
#undef debugf
