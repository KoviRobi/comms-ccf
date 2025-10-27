/**

# Type utilities

When we have some compile-time known value, it is useful to know what
is the smallest type required to hold it.

*/
#pragma once

#include <stddef.h>
#include <stdint.h>

#include <bit>

template<size_t N>
struct Uint{};
template<> struct Uint<8> { using Type = uint8_t; };;
template<> struct Uint<16> { using Type = uint16_t; };;
template<> struct Uint<32> { using Type = uint32_t; };;
template<> struct Uint<64> { using Type = uint64_t; };;
template<size_t N> using UintT = Uint<N>::Type;

template<size_t I>
struct SmallestType
{
    static constexpr unsigned _bits = std::bit_width(I);
    static constexpr unsigned _bytes = (_bits + 7) / 8;
    static constexpr unsigned N = std::bit_ceil(_bytes);
    using Type = Uint<N>;
};
template<size_t I> using SmallestTypeT = SmallestType<I>::Type;
