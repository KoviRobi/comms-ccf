/**

# Type utilities

When we have some compile-time known value, it is useful to know what
is the smallest type required to hold it.

*/
#pragma once

#include <stddef.h>
#include <stdint.h>

template<size_t I>
struct SmallestType{};

template<size_t I> requires (I <= 0xFF)
struct SmallestType<I> { using Type = uint8_t; };

template<size_t I> requires (0xFF < I && I <= 0xFFFF)
struct SmallestType<I> { using Type = uint16_t; };

template<size_t I> requires (0xFFFF < I && I <= 0xFFFF'FFFF)
struct SmallestType<I> { using Type = uint32_t; };

template<size_t I> requires (0xFFFF'FFFF < I && I <= 0xFFFF'FFFF'FFFF'FFFF)
struct SmallestType<I> { using Type = uint64_t; };

template<size_t I>
using SmallestTypeT = SmallestType<I>::Type;
