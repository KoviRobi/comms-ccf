/**

# Concise Binary Object Representation (CBOR)

We need a simple, standardised (makes implementing in other languages
easier) way to serialise and deserialise data. There's a couple of approaches:

1. Define a protocol at the byte level, works well but manual.
2. Define a protocol in a DSL/schema and let that define the byte level, which
   works well but can have issues with backwards compatibility when the
   protocol is extended. Examples of this are Cap'n Proto, ProtoBuf,
   ASN.1.
3. Text-based generic/schemaless encodings such as JSON and XML,
   unnecessary overhead for embedded devices.
4. Byte-based variants of JSON/XML, which is what we are going for.
   Some alternatives are:
   1. BSON, a binary variant of JSON, which is designed for being
      modifiable rather than compact.
   2. MessagePack which seems good, but compared to CBOR is limited
      (doesn't support tagging). Though it would probably be sufficient
      here, I have gone for CBOR in case it comes in useful in the future.
   3. CBOR, which is what I have gone for. This is inspired by
      MessagePack, and has a description which is reasonably simple
      to understand/implement.

## High-level overview

The CBOR format has a consistent encoding across the different data-types,
which makes implementation simple. The *initial byte* of a CBOR value is
composed of:

	[](u3 major, u5 minor) { return ((major << 5) | minor); }

The major types are (from [RFC8949 table
1](https://www.rfc-editor.org/rfc/rfc8949.html#major-type-table)):

| Major Type | Meaning               | Content                         |
|------------|-----------------------|---------------------------------|
| 0          | unsigned integer N    | -                               |
| 1          | negative integer -1-N | -                               |
| 2          | byte string           | N bytes                         |
| 3          | text string           | N bytes (UTF-8 text)            |
| 4          | array                 | N data items (elements)         |
| 5          | map                   | 2N data items (key/value pairs) |
| 6          | tag of number N       | 1 data item                     |
| 7          | simple/float          | -                               |

And the value has the following forms (adapted from [RFC8949 table
3](https://www.rfc-editor.org/rfc/rfc8949.html#fpnoconttbl)):

| 5-Bit Minor | Semantics                                         |
|-------------|---------------------------------------------------|
| 0..23/0x17  | Simple value (value 0..23)                        |
| 24/0x18     | Simple value (value 32..255 in following byte)    |
| 25/0x19     | A 16 bit value                                    |
| 26/0x1A     | A 32 bit value                                    |
| 27/0x1B     | A 64 bit value                                    |
| 28-30/C/D/E | Reserved, not well-formed in the present document |
| 31/0x1F     | Start/stop indefinite length values               |

Floating-point values are either 16/32/64 bit values, the major 7
minor values 0-255 are simple values. Indefinite length values are
strings/arrays/maps without a length known ahead of time. They are started
by e.g. an array with minor value 31, and are ended with a simple/float
of minor value 31. In the case of strings, it is a concatenation of
definite length strings

The table of simple values from [RFC8949 table
4](https://www.rfc-editor.org/rfc/rfc8949.html#fpnoconttbl2)

| Value   | Semantics    |
|---------|--------------|
| 0..19   | (unassigned) |
| 20      | false        |
| 21      | true         |
| 22      | null         |
| 23      | undefined    |
| 24..31  | (reserved)   |
| 32..255 | (unassigned) |

## Examples

I will be using the byte-string syntax `b"\1\x0A"` to denote a sequence
of byte values in hexadecimal, so this is bytes `1, 10`. The syntax
`N(X)` denotes an item `X` that has an integer tag `N`.

Some examples to aid, from [RFC8949 Appendix
A](https://www.rfc-editor.org/rfc/rfc8949.html#section-appendix.a)
(apologies about the wide table, the test/cbor.cpp code parses it so
for simplicity I didn't split the rows up, but if this is rendered as
HTML that doesn't matter):

*Marker for test/cbor.cpp*

| Decoded                                                                                       | Encoded (initial byte, hexdump)                                    | Type                              |
|-----------------------------------------------------------------------------------------------|--------------------------------------------------------------------|-----------------------------------|
| 0                                                                                             | `0:0`  ``                                                          | uint64_t                          |
| 1                                                                                             | `0:1`  ``                                                          | uint64_t                          |
| 10                                                                                            | `0:10` ``                                                          | uint64_t                          |
| 23                                                                                            | `0:23` ``                                                          | uint64_t                          |
| 24                                                                                            | `0:24` `18`                                                        | uint64_t                          |
| 25                                                                                            | `0:24` `19`                                                        | uint64_t                          |
| 100                                                                                           | `0:24` `64`                                                        | uint64_t                          |
| 1000                                                                                          | `0:25` `03E8`                                                      | uint64_t                          |
| 1000000                                                                                       | `0:26` `000F4240`                                                  | uint64_t                          |
| 1000000000000                                                                                 | `0:27` `000000E8D4A51000`                                          | uint64_t                          |
| 18446744073709551615                                                                          | `0:27` `FFFFFFFFFFFFFFFF`                                          | uint64_t                          |
| 18446744073709551616                                                                          | `6:2`  `49010000000000000000`                                      | Skip (value too large for 64_t)   |
| -18446744073709551616                                                                         | `1:27` `FFFFFFFFFFFFFFFF`                                          | Skip (value too large for 64_t)   |
| -18446744073709551617                                                                         | `6:3`  `49010000000000000000`                                      | Skip (value too large for 64_t)   |
| -1                                                                                            | `1:0`  ``                                                          | int64_t                           |
| -10                                                                                           | `1:9`  ``                                                          | int64_t                           |
| -100                                                                                          | `1:24` `63`                                                        | int64_t                           |
| -1000                                                                                         | `1:25` `03E7`                                                      | int64_t                           |
| 0.0                                                                                           | `7:25` `0000`                                                      | _Float16                          |
| -0.0                                                                                          | `7:25` `8000`                                                      | _Float16                          |
| 1.0                                                                                           | `7:25` `3C00`                                                      | _Float16                          |
| 1.1                                                                                           | `7:27` `3FF199999999999A`                                          | double                            |
| 1.5                                                                                           | `7:25` `3E00`                                                      | _Float16                          |
| 65504.0                                                                                       | `7:25` `7BFF`                                                      | _Float16                          |
| 100000.0                                                                                      | `7:26` `47C35000`                                                  | float                             |
| 3.4028234663852886e+38                                                                        | `7:26` `7F7FFFFF`                                                  | float                             |
| 1.0e+300                                                                                      | `7:27` `7E37E43C8800759C`                                          | double                            |
| 5.960464477539063e-8                                                                          | `7:25` `0001`                                                      | _Float16                          |
| 0.00006103515625                                                                              | `7:25` `0400`                                                      | _Float16                          |
| -4.0                                                                                          | `7:25` `C400`                                                      | _Float16                          |
| -4.1                                                                                          | `7:27` `C010666666666666`                                          | double                            |
| Infinity                                                                                      | `7:25` `7C00`                                                      | _Float16                          |
| NaN                                                                                           | `7:25` `7E00`                                                      | _Float16                          |
| -Infinity                                                                                     | `7:25` `FC00`                                                      | _Float16                          |
| Infinity                                                                                      | `7:26` `7F800000`                                                  | float                             |
| NaN                                                                                           | `7:26` `7FC00000`                                                  | float                             |
| -Infinity                                                                                     | `7:26` `FF800000`                                                  | float                             |
| Infinity                                                                                      | `7:27` `7FF0000000000000`                                          | double                            |
| NaN                                                                                           | `7:27` `7FF8000000000000`                                          | double                            |
| -Infinity                                                                                     | `7:27` `FFF0000000000000`                                          | double                            |
| false                                                                                         | `7:20` ``                                                          | bool                              |
| true                                                                                          | `7:21` ``                                                          | bool                              |
| null                                                                                          | `7:22` ``                                                          | void *                            |
| undefined                                                                                     | `7:23` ``                                                          | N/A                               |
| simple(16)                                                                                    | `7:16` ``                                                          | N/A                               |
| simple(255)                                                                                   | `7:24` `FF`                                                        | N/A                               |
| 0("2013-03-21T20:04:00Z")                                                                     | `6:0`  `74323031332D30332D32315432303A 30343A30305A`               | N/A                               |
| 1(1363896240)                                                                                 | `6:1`  `1A514B67B0`                                                | N/A                               |
| 1(1363896240.5)                                                                               | `6:1`  `FB41D452D9EC200000`                                        | N/A                               |
| 23(h'01020304')                                                                               | `6:23` `4401020304`                                                | N/A                               |
| 24(h'6449455446')                                                                             | `6:24` `18456449455446`                                            | N/A                               |
| 32("http://www.example.com")                                                                  | `6:24` `2076687474703A2F2F7777772E6578 616D706C652E636F6D`         | N/A                               |
| h''                                                                                           | `2:0`  ``                                                          | std::span<uint8_t>                |
| h'01020304'                                                                                   | `2:4`  `01020304`                                                  | std::span<uint8_t>                |
| h'31323334'                                                                                   | `2:4`  `31323334`                                                  | std::span<uint8_t>                |
| ""                                                                                            | `3:0`  ``                                                          | std::string_view                  |
| "a"                                                                                           | `3:1`  `61`                                                        | std::string_view                  |
| "IETF"                                                                                        | `3:4`  `49455446`                                                  | std::string_view                  |
| "\""                                                                                          | `3:2`  `225C`                                                      | Skip (not parsing escapes)        |
| "\u00fc"                                                                                      | `3:2`  `C3BC`                                                      | Skip (not parsing escapes)        |
| "\u6c34"                                                                                      | `3:3`  `E6B0B4`                                                    | Skip (not parsing escapes)        |
| "\ud800\udd51"                                                                                | `3:4`  `F0908591`                                                  | Skip (not parsing escapes)        |
| []                                                                                            | `4:0`  ``                                                          | std::span<int>                    |
| [1, 2, 3]                                                                                     | `4:3`  `010203`                                                    | std::span<int>                    |
| [1, [2, 3], [4, 5]]                                                                           | `4:3`  `01820203820405`                                            | tuple<int, span<int>, span<int>>  |
| [1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25]   | `4:24` `190102030405060708090A0B0C0D0E 0F101112131415161718181819` | std::span<int>                    |
| {}                                                                                            | `5:0`  ``                                                          | N/A                               |
| {1: 2, 3: 4}                                                                                  | `5:2`  `01020304`                                                  | N/A                               |
| {"a": 1, "b": [2, 3]}                                                                         | `5:2`  `6161016162820203`                                          | N/A                               |
| ["a", {"b": "c"}]                                                                             | `4:2`  `6161A161626163`                                            | N/A                               |
| {"a": "A", "b": "B", "c": "C", "d": "D", "e": "E"}                                            | `5:5`  `616161416162614261636143616461 4461656145`                 | N/A                               |
| (_ h'0102', h'030405')                                                                        | `2:31` `42010243030405FF`                                          | N/A                               |
| (_ "strea", "ming")                                                                           | `3:31` `657374726561646D696E67FF`                                  | N/A                               |
| [_ ]                                                                                          | `4:31` `FF`                                                        | N/A                               |
| [_ 1, [2, 3], [_ 4, 5]]                                                                       | `4:31` `018202039F0405FFFF`                                        | N/A                               |
| [_ 1, [2, 3], [4, 5]]                                                                         | `4:31` `01820203820405FF`                                          | N/A                               |
| [1, [2, 3], [_ 4, 5]]                                                                         | `4:3`  `018202039F0405FF`                                          | N/A                               |
| [1, [_ 2, 3], [4, 5]]                                                                         | `4:3`  `019F0203FF820405`                                          | N/A                               |
| [_ 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25] | `4:31` `0102030405060708090A0B0C0D0E0F 101112131415161718181819FF` | N/A                               |
| {_ "a": 1, "b": [_ 2, 3]}                                                                     | `5:31` `61610161629F0203FFFF`                                      | N/A                               |
| ["a", {_ "b": "c"}]                                                                           | `4:2`  `6161BF61626163FF`                                          | N/A                               |
| {_ "Fun": true, "Amt": -2}                                                                    | `5:31` `6346756EF563416D7421FF`                                    | N/A                               |

*/

#pragma once

#include "types.hpp"

#ifndef __STDC_WANT_IEC_60559_TYPES_EXT__
#define __STDC_WANT_IEC_60559_TYPES_EXT__
#endif
#include <float.h>
#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <array>
#include <bit>
#include <concepts>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

namespace Cbor
{

#if (defined(FLT16_MIN) || defined(FLT16_MIN)) && !defined(NO_FLOAT16)
#define CBOR_HALF_SUPPORT
#endif

#if defined(CBOR_HALF_SUPPORT)
typedef _Float16 half;
#endif

    /// Upper three bits of the initial/header byte
    enum class Major : uint8_t
    {
        /// Value is N the positive integer
        U64 = 0,
        /// Value is -N-1 (note this is ~N in 2s complement)
        Neg64 = 1,
        /// Value is the number of bytes that follow, unescaped. Or the
        /// indefinite sequence of zero or more byte chunks (bytes with
        /// specified length) concatenated together.
        Bytes = 2,
        /// Value is the number of bytes (not code-points) that follow,
        /// unescaped.  Or the indefinite sequence of zero or more UTF-8
        /// chunks (bytes with specified length) concatenated together
        Utf8 = 3,
        /// Value is number of items that follow, or indefinite sequence
        /// of items.
        Array = 4,
        /// Value is number of key-value pairs that follow, or indefinite
        /// sequence of items.
        Map = 5,
        /// Value is the tag, followed by a single item.
        Tagged = 6,
        /// Simple value (embedded or 8b), float (16b, 32b, 64b), or
        /// indefinite sequence.
        Simple = 7,
        Float = 7,
    };
    constexpr Major getMajor(uint8_t initial) { return Major(initial >> 5); }
    constexpr uint8_t setMajor(uint8_t initial, Major major)
    {
        return static_cast<uint8_t>(
            (initial & ~(~0 << 5)) |
            static_cast<uint8_t>(major) << 5);
    }

    enum class Minor : uint8_t
    {
        /// Embedded means no next byte, value is contained within.
        EmbeddedFirst = 0,
        EmbeddedLast = 23,
        /// These contain the next value in the following byte(s).
        OneByteFollows = 24,
        TwoByteFollows = 25,
        FourByteFollows = 26,
        EightByteFollows = 27,
        /// Bytes/Utf8/Array/Map+Indefinite starts an indefinite-length
        /// item. Simple/Float+Indefinite is a terminator for
        /// indefinite-length items.
        Indefinite = 31,
    };
    constexpr uint8_t EMBEDDED_MIN = static_cast<uint8_t>(Minor::EmbeddedFirst);
    constexpr uint8_t EMBEDDED_MAX = static_cast<uint8_t>(Minor::EmbeddedLast);
    constexpr Minor getMinor(uint8_t initial) { return Minor(initial & ((1 << 5) - 1)); }
    constexpr uint8_t setMinor(uint8_t initial, Minor minor)
    {
        return static_cast<uint8_t>(
            (initial & (~0 << 5)) |
            static_cast<uint8_t>(minor));
    }
    constexpr uint8_t initialByte(Major major, Minor minor)
    {
        return setMinor(setMajor(0, major), minor);
    }
    consteval Minor minorFromSizeOf(const size_t bytes)
    {
        return
            bytes == 1 ? Minor::OneByteFollows :
            bytes == 2 ? Minor::TwoByteFollows :
            bytes == 4 ? Minor::FourByteFollows :
            Minor::EightByteFollows;
     }

    /// Possible values for Minor when Major is Simple/Float
    enum class SimpleValues : uint8_t
    {
        Unassigned0First = 0,
        Unassigned0Last = 19,
        False = 20,
        True = 21,
        Null = 22,
        Undefined = 23,
        ReservedFirst = 24,
        ReservedLast = 31,
        Unassigned1First = 32,
        Unassigned1Last = 255,
    };

    /// Pack a value [0, 23] embedded into the first header byte.
    bool packEmbedded(Major major, uint8_t value, std::span<uint8_t> & buf);
    /// Pack an N bit value (usually if the value is [0, 23] you
    /// want packEmbedded).  Ends up as an initial header byte and
    /// ceil(N/8) value bytes
    template<std::unsigned_integral Int>
    bool pack(Major major, Int value, std::span<uint8_t> & buf);
    template<>
    bool pack<unsigned char>(Major major, unsigned char value, std::span<uint8_t> & buf);
    template<>
    bool pack<unsigned short>(Major major, unsigned short value, std::span<uint8_t> & buf);
    template<>
    bool pack<unsigned int>(Major major, unsigned int value, std::span<uint8_t> & buf);
    template<>
    bool pack<unsigned long>(Major major, unsigned long value, std::span<uint8_t> & buf);
    template<>
    bool pack<unsigned long long>(Major major, unsigned long long value, std::span<uint8_t> & buf);

    /// A decoded CBOR item (though some values e.g. bytes/strings just need to be
    /// copied out)
    struct Item
    {
        Major major;
        Minor minor;
        uint64_t value;
    };

    /// Unpack one item
    std::optional<Item> unpack(std::span<uint8_t> & buf);

    /// Encodes a value that can be up to N bits. Unlike \see pack<T>,
    /// it uses the smallest encoding.
    template<std::unsigned_integral Int>
    bool encode(Major major, Int value, std::span<uint8_t> & buf);
    template<>
    bool encode<unsigned char>(Major major, unsigned char value, std::span<uint8_t> & buf);
    template<>
    bool encode<unsigned short>(Major major, unsigned short value, std::span<uint8_t> & buf);
    template<>
    bool encode<unsigned int>(Major major, unsigned int value, std::span<uint8_t> & buf);
    template<>
    bool encode<unsigned long>(Major major, unsigned long value, std::span<uint8_t> & buf);
    template<>
    bool encode<unsigned long long>(Major major, unsigned long long value, std::span<uint8_t> & buf);

    /// Template functionality

    template<typename T>
    struct Cbor
    {
        /*
         * Interface to this template:

        static bool encode(T value, std::span<uint8_t> & buf);
        static std::optional<T> decode(std::span<uint8_t> & buf);
        static size_t maxSize();

         * Though C++ doesn't require this and in fact is better at giving
         * compile-errors (rather than link errors) if it is commented out, as then
         * it doesn't assume it is defined in a different object.
         */
    };

    template<typename I> requires std::integral<I> && (!std::same_as<I, bool>)
    struct Cbor<I>
    {
        static constexpr size_t bytes = sizeof(I);
        static bool encode(I value, std::span<uint8_t> & buf)
        {
            Major major = value >= 0 ? Major::U64 : Major::Neg64;
            /// Note, in 2s complement -n-1 is bitwise negated n (~n)
            value = value >= 0 ? value : ~value;
            using UI = std::make_unsigned_t<I>;
            return ::Cbor::encode<UI>(major, std::bit_cast<UI>(value), buf);
        }
        static std::optional<I> decode(std::span<uint8_t> & buf)
        {
            auto value = unpack(buf);
            if (!value)
            {
                return {};
            }
            if (value->major == Major::Neg64)
            {
                /// Note, in 2s complement -n-1 is bitwise negated n (~n)
                value->value = ~value->value;
            }
            else if (value->major != Major::U64)
            {
                return {};
            }
            if (value->minor <= minorFromSizeOf(bytes))
            {
                return {value->value};
            }
            return {};
        }
        static size_t maxSize()
        {
            return 1 + bytes;
        }
    };

    template<typename F> requires std::floating_point<F>
#if defined(CBOR_HALF_SUPPORT)
        || std::same_as<F, _Float16>
#endif
    struct Cbor<F>
    {
        static constexpr size_t bytes = sizeof(F);
        using BitT = UintT<bytes * 8>;
        static bool encode(F value, std::span<uint8_t> & buf)
        {
            // TODO: For deterministically encoded CBOR we would need
            // to select the smallest float encoding
            BitT bits = std::bit_cast<BitT>(value);
            return ::Cbor::pack<BitT>(Major::Float, bits, buf);
        }
        static std::optional<F> decode(std::span<uint8_t> & buf)
        {
            auto value = unpack(buf);
            if (!value || value->major != Major::Float)
            {
                return {};
            }
            // Allow upcasts to support deterministically encoded CBOR
#if defined(CBOR_HALF_SUPPORT)
            if (value->minor == Minor::TwoByteFollows && 2 <= bytes)
            {
                return {static_cast<F>(std::bit_cast<_Float16>(static_cast<uint16_t>(value->value)))};
            }
#endif
            if (value->minor == Minor::FourByteFollows && 4 <= bytes)
            {
                return {static_cast<F>(std::bit_cast<float>(static_cast<uint32_t>(value->value)))};
            }
            if (value->minor == Minor::EightByteFollows && 8 <= bytes)
            {
                return {static_cast<F>(std::bit_cast<double>(static_cast<uint64_t>(value->value)))};
            }
            return {};
        }
        static size_t maxSize()
        {
            return 1 + bytes;
        }
    };

    template<>
    struct Cbor<bool>
    {
        static bool encode(bool value, std::span<uint8_t> & buf)
        {
            return packEmbedded(
                Major::Simple,
                static_cast<uint8_t>(value ? SimpleValues::True : SimpleValues::False),
                buf);
        }
        static std::optional<bool> decode(std::span<uint8_t> & buf)
        {
            auto value = unpack(buf);
            if (value.has_value() && value->major == Major::Simple)
            {
                if (value->minor == static_cast<Minor>(SimpleValues::True))
                {
                    return {true};
                }
                else if (value->minor == static_cast<Minor>(SimpleValues::False))
                {
                    return {false};
                }
            }
            return {};
        }
    };

    template<typename T>
    struct Cbor<T *>
    {
        static bool encode(T * obj, std::span<uint8_t> & buf)
        {
            return obj != nullptr
                ? Cbor<T>::encode(*obj, buf)
                : packEmbedded(Major::Simple, static_cast<uint8_t>(SimpleValues::Null), buf);
        }
    };

    template<>
    struct Cbor<std::string_view>
    {
        static bool encode(std::string_view str, std::span<uint8_t> & buf)
        {
            if (!::Cbor::encode<size_t>(
                Major::Utf8, str.size(), buf))
            {
                return false;
            }
            for (char c : str)
            {
                if (buf.size() == 0)
                {
                    return false;
                }
                buf[0] = c;
                buf = buf.subspan(1);
            }
            return true;
        }
    };

    template<typename... Item>
    struct Cbor<std::tuple<Item...>>
    {
        using Tuple = std::tuple<Item...>;
        static bool encode(Tuple tup, std::span<uint8_t> & buf)
        {
            if (buf.size() < 1)
            {
                return false;
            }
            return
                ::Cbor::encode<size_t>(
                    Major::Array,
                    std::tuple_size_v<Tuple>,
                    buf) &&
                [&]<size_t... Index>(std::index_sequence<Index...>)
                {
                    return (
                        Cbor<std::tuple_element_t<Index, Tuple>>::encode(std::get<Index>(tup), buf) &&
                        ...
                    );
                }(std::index_sequence_for<Item...>{});
        }
        template<typename... T>
        static std::optional<std::tuple<T...>> transpose(std::tuple<std::optional<T>...> ts)
        {
            return [&]<size_t... Index>(std::index_sequence<Index...>)
            {
                if ((std::get<Index>(ts) && ...))
                {
                    return std::optional<std::tuple<T...>>{std::in_place, *std::get<Index>(ts)...};
                }
                return std::optional<std::tuple<T...>>{};
            }(std::index_sequence_for<T...>{});
        }
        static std::optional<Tuple> decode(std::span<uint8_t> & buf)
        {
            auto value = unpack(buf);
            if (value.has_value() && value->major == Major::Array)
            {
                std::tuple<std::optional<Item>...> ts{Cbor<Item>::decode(buf)...};
                if (value->minor != Minor::Indefinite)
                {
                    return transpose(ts);
                }
                else
                {
                    auto sentinel = unpack(buf);
                    if (sentinel.has_value() &&
                        sentinel->major == Major::Simple &&
                        sentinel->minor == Minor::Indefinite)
                    {
                        return transpose(ts);
                    }
                }
            }
            return {};
        }
    };

    template<size_t Size>
    struct Cbor<std::span<uint8_t, Size>>
    {
        static bool encode(std::span<uint8_t, Size> span, std::span<uint8_t> & buf)
        {
            if (buf.size() < 1)
            {
                return false;
            }
            if (!::Cbor::encode<size_t>(Major::Bytes, span.size(), buf))
            {
                return false;
            }
            if (buf.size() < span.size())
            {
                return false;
            }
            std::copy(span.cbegin(), span.cend(), buf.begin());
            buf = buf.subspan(span.size());
            return true;
        }
        static std::optional<std::span<uint8_t, Size>> decode(std::span<uint8_t> & buf)
        {
            auto value = unpack(buf);
            if (value.has_value() && value->major == Major::Bytes)
            {
                if (value->minor == Minor::Indefinite)
                {
                    return {};
                }
                else
                {
                    return {buf};
                }
            }
            return {};
        }
    };

    template<typename T, size_t Size>
    struct Cbor<std::array<T, Size>>
    {
        static bool encode(std::array<T, Size> array, std::span<uint8_t> & buf)
        {
            if (buf.size() < 1)
            {
                return false;
            }
            if (!::Cbor::encode<size_t>(Major::Array, array.size(), buf))
            {
                return false;
            }
            for (const auto & item : array)
            {
                if (!Cbor<T>::encode(item, buf))
                {
                    return false;
                }
            }
            return true;
        }
        static std::optional<std::array<T, Size>> decode(std::span<uint8_t> & buf)
        {
            auto value = unpack(buf);
            if (value.has_value() && value->major == Major::Array)
            {
                std::array<T, Size> array =
                    [&]<size_t... Index>(std::index_sequence<Index...>)
                    {
                        return std::array<T, Size>{
                            *((void)Index, Cbor<T>::decode(buf))...
                        };
                    }(std::make_index_sequence<Size>{});
                if (value->minor != Minor::Indefinite)
                {
                    return array;
                }
                else
                {
                    auto sentinel = unpack(buf);
                    if (sentinel.has_value() &&
                        sentinel->major == Major::Simple &&
                        sentinel->minor == Minor::Indefinite)
                    {
                        return array;
                    }
                }
            }
            return {};
        }
    };

    template<Major major>
    class Sequence
    {
    public:
        Sequence(std::span<uint8_t> & buf_, size_t extent_ = std::dynamic_extent)
            : buf(buf_),  extent(extent_)
        {
            if (extent == std::dynamic_extent)
            {
                packEmbedded(major, static_cast<uint8_t>(Minor::Indefinite), buf);
            }
            else
            {
                ::Cbor::encode<size_t>(major, extent, buf);
            }
        }
        template<Major parentMajor>
        Sequence(Sequence<parentMajor> & parentSeq, size_t extent_ = std::dynamic_extent)
            : buf(parentSeq.buf),  extent(extent_)
        {
            ++parentSeq.packed;
            if (extent == std::dynamic_extent)
            {
                packEmbedded(major, static_cast<uint8_t>(Minor::Indefinite), buf);
            }
            else
            {
                ::Cbor::encode<size_t>(major, extent, buf);
            }
        }
        bool as_expected()
        {
            return extent == std::dynamic_extent || extent == packed;
        }
        ~Sequence()
        {
            if (extent == std::dynamic_extent)
            {
                packEmbedded(Major::Simple, static_cast<uint8_t>(Minor::Indefinite), buf);
            }
        }

        template<typename T>
        bool encode(T value)
        {
            ++packed;
            return Cbor<T>::encode(value, buf);
        }
    private:
        std::span<uint8_t> & buf;
        const size_t extent;
        size_t packed = 0;
    };
};
