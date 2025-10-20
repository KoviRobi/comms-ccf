/**

This code parses the markdown table in comms-ccf/cbor.hpp, which has the following format

    *Marker for test/cbor.cpp*

    | Decoded | Encoded (initial byte, hexdump) | Type     |
    |---------|---------------------------------|----------|
    | 0       | `0:0`  ``                       | uint64_t |
    | 23      | `0:23` ``                       | uint64_t |
    | 24      | `0:24` `18`                     | uint64_t |
    | 100     | `0:24` `64`                     | uint64_t |
    | 1000    | `0:25` `03E8`                   | uint64_t |
    | 1000000 | `0:26` `000F4240`               | uint64_t |

To do this, it first finds the marker, then parses each row, extracts
the values and type, and for the types implemented it check encode/decode
behaves as expected.

There are a few parts to this code:
- Extended equals: normally NaN != NaN
- Parsing the encoded/decoded into values
- Dispatching on the type column

*/
#include "cbor.hpp"

#include <stdint.h>
#include <stdio.h>

#include <algorithm>
#include <charconv>
#include <cmath>
#include <concepts>
#include <fstream>
#include <iostream>
#include <optional>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>

using namespace std::literals;

constexpr std::string_view RED = "\x1b[31m";
constexpr std::string_view YELLOW = "\x1b[33m";
constexpr std::string_view RESET = "\x1b[0m";

/// Equality also having NaN==NaN
template<typename T>
bool eq(T a, T b) requires (requires(T t) { std::isnan(t); })
{
    return (a == b) || (std::isnan(a) && std::isnan(b));
}
template<std::ranges::range T>
bool eq(T a, T b) requires (!requires(T t) { std::isnan(t); })
{
    return std::ranges::equal(a, b);
}
template<typename T>
bool eq(T a, T b) requires (!requires(T t) { std::isnan(t); } && !std::ranges::range<T>)
{
    return a == b;
}

template<typename T>
struct Parse { static std::optional<T> parse(std::string s); };

/// Parse an integral number, returning nullopt on failure.
template<typename I> requires std::integral<I> && (!std::same_as<I, bool>)
struct Parse<I>
{
    static std::optional<I> parse(std::string s, int base = 10)
    {
        I integral;
        auto res = std::from_chars(&*s.cbegin(), &*s.cend(), integral, base);
        return (res.ec == std::errc{}) ? std::optional<I>{integral} : std::nullopt;
    }
};

/// Parse a floating point number, returning nullopt on failure.
template<typename F> requires std::floating_point<F>
#if defined(CBOR_HALF_SUPPORT)
    || std::same_as<F, _Float16>
#endif
struct Parse<F>
{
    static std::optional<F> parse(std::string s)
    {
        F fp;
        auto res = std::from_chars(&*s.cbegin(), &*s.cend(), fp);
        return (res.ec == std::errc{}) ? std::optional{fp} : std::nullopt;
    }
};

/// Parse a boolean, returning nullopt on failure.
template<typename B> requires std::same_as<B, bool>
struct Parse<B>
{
    static std::optional<bool> parse(std::string s)
    {
        return s == "true" ? std::optional{true} :
               s == "false" ? std::optional{false} : std::nullopt;
    }
};

/// Parse a sequence of bytes, with optional space padding
static bool parse_bytes(std::string s, std::vector<uint8_t> & into)
{
    std::regex byte{"\\s*([0-9a-fA-F]{2})"};
    auto it = std::sregex_iterator{s.cbegin(), s.cend(), byte};
    decltype(it) last;
    for (; it != last; ++it)
    {
        if (it->size() != 2) break;
        auto b = Parse<uint8_t>::parse((*it)[1], 16);
        if (!b) break;
        into.push_back(*b);
    }
    if (it != last)
    {
        std::cout << "Error in parsing bytes " << s;
        return false;
    }
    return true;
}

/// Parse a byte string (sequence of bytes/base16)
template<>
struct Parse<std::span<uint8_t>>
{
    static std::optional<std::span<uint8_t>> parse(std::string s)
    {
        static std::vector<uint8_t> into;
        into.clear();
        std::regex literal{"h'((?:[0-9a-fA-F]{2})*)'"};
        std::smatch m;
        if (
            std::regex_match(s.cbegin(), s.cend(), m, literal) &&
            parse_bytes(m[1], into)
        )
        {
            return {into};
        }
        return {};
    }
};

static std::ostream & operator<<(std::ostream & os, const std::span<uint8_t> span)
{
    auto fill = os.fill();
    auto width = os.width();
    for (uint8_t byte : span)
    {
        os.fill('0');
        os.width(2);
        os << byte;
    }
    os.fill(fill);
    os.width(width);
    return os;
}

int main(int argc, char ** argv)
{
    bool ok = true;

    const char * path = "comms-ccf/cbor.hpp";
    if (argc == 2)
    {
        path = argv[1];
    }
    std::cout << "Using path " << path << "\n";
    std::ifstream file(path);
    std::string line;
    while (std::getline(file, line))
    {
        if (line == "*Marker for test/cbor.cpp*"sv)
            break;
    }

    std::getline(file, line); // <empty line>
    std::getline(file, line); // | Decoded | Encoded | Type |
    std::getline(file, line); // |---------|---------|------|

    #define NUM R"-(((?:0[xX])?[0-9a-fA-F]+))-" // Number with optional hex base
    #define BYTES R"-(((?:[0-9a-fA-F]{2}|\s)*))-" // Pair of nibbles with optional spaces
    #define CELL R"-(\s+([^|]*?)\s+)-" // Anything, trimming the spaces at end
    std::regex values{"\\|" CELL "\\|\\s+(`" NUM ":" NUM "`\\s+`" BYTES "`)\\s+\\|" CELL "\\|"};
    enum {All, Decoded, Encoded, MajorNum, MinorNum, Bytes, Type};

    while (std::getline(file, line))
    {
        std::smatch m;
        if (!std::regex_match(line, m, values))
        {
            if (line == ""sv)
            {
                break;
            }
            std::cout << YELLOW << "Error in line " << line << RESET << "\n";
            ok = false;
        }
        auto major = Parse<uint8_t>::parse(m[MajorNum]);
        auto minor = Parse<uint8_t>::parse(m[MinorNum]);
        if (!major || !minor)
        {
            std::cout << YELLOW << "Failed to parse initial byte "
                      << m[MajorNum] << ":" << m[MinorNum]
                      << " in " << line << RESET << "\n";
            ok = false;
            continue;
        }
        std::vector<uint8_t> bytes;
        bytes.push_back( (*major<<5) | *minor );
        if (!parse_bytes(m[Bytes], bytes))
        {
            std::cout << YELLOW << "Failed to parse bytes "
                      << m[Bytes] << " in " << line << RESET << "\n";
            ok = false;
            continue;
        }
        std::span encoded{bytes};

        // Dispatch the test on each type
#define DISPATCH(TYPE)                                                               \
        if (m[Type] == #TYPE)                                                        \
        {                                                                            \
            std::optional<TYPE> expected = Parse<TYPE>::parse(m[Decoded]);           \
            std::optional<TYPE> decoded = Cbor::Cbor<TYPE>::decode(encoded);         \
            if (!decoded || !expected || !eq(*decoded, *expected))                   \
            {                                                                        \
                std::cerr << RED << "Failed to decode " << m[Encoded] << "\tinto\t"  \
                          << *expected << "\tgot\t" << *decoded << RESET << "\n";    \
                ok = false;                                                          \
                continue;                                                            \
            }                                                                        \
            std::vector<uint8_t> buf(bytes.size(), 0);                               \
            std::span encodedSpan{buf};                                              \
            auto encOk = Cbor::Cbor<TYPE>::encode(*expected, encodedSpan);           \
            if (!encOk || !std::ranges::equal(bytes, buf))                           \
            {                                                                        \
                std::cerr << RED << "Failed to encode " << m[Decoded] << "\tinto\t"; \
                for (auto c : bytes)                                                 \
                    fprintf(stderr, "%02X", c);                                      \
                fprintf(stderr, "\tgot\t");                                          \
                for (auto s = &*buf.begin(); s != &*encodedSpan.begin(); ++s)        \
                    fprintf(stderr, "%02X", *s);                                     \
                std::cerr << RESET << "\n";                                          \
                ok = false;                                                          \
            }                                                                        \
        }
        DISPATCH(uint64_t)
        else DISPATCH(int64_t)
        else DISPATCH(bool)
#if defined(CBOR_HALF_SUPPORT)
        else DISPATCH(_Float16)
#endif
        else DISPATCH(float)
        else DISPATCH(double)
        else DISPATCH(std::span<uint8_t>)
        else if (m[Type] != "N/A" && m[Type].str().substr(0, 4) != "Skip")
        {
            std::cout << YELLOW << "Don't know how to test "
                      << "| " << m[Type] << " | " << m[Decoded] << " | " << m[Encoded] << " |"
                      << RESET << "\n";
        }
    }
    return ok ? 0 : 1;
}
