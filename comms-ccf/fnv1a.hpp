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
    constexpr uint32_t checksum(std::span<uint8_t, Size> span)
    {
        uint32_t hash = initialHash;
        for (auto c : span)
        {
            hash = feed(hash, c);
        }
        return hash;
    }
};
