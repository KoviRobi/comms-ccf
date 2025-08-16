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
#include <regex>
#include <string>
#include <string_view>

using namespace std::literals;

/// Equality also having NaN==NaN
template<typename T>
bool eq(T a, T b) requires (requires(T t) { std::isnan(t); })
{
    return (a == b) || (std::isnan(a) && std::isnan(b));
}
template<typename T>
bool eq(T a, T b) requires (!requires(T t) { std::isnan(t); })
{
    return a == b;
}

/// Parse an integral number, returning nullopt on failure.
template<typename I> requires std::integral<I> && (!std::same_as<I, bool>)
static std::optional<I> parse(std::string s, int base = 10)
{
    I integral;
    auto res = std::from_chars(&*s.cbegin(), &*s.cend(), integral, base);
    return (res.ec == std::errc{}) ? std::optional<I>{integral} : std::nullopt;
}

/// Parse a floating point number, returning nullopt on failure.
template<typename F> requires std::floating_point<F>
#if defined(CBOR_HALF_SUPPORT)
    || std::same_as<F, _Float16>
#endif
static std::optional<F> parse(std::string s)
{
    F fp;
    auto res = std::from_chars(&*s.cbegin(), &*s.cend(), fp);
    return (res.ec == std::errc{}) ? std::optional{fp} : std::nullopt;
}

/// Parse a boolean, returning nullopt on failure.
template<typename B> requires std::same_as<B, bool>
static std::optional<bool> parse(std::string s)
{
    return s == "true" ? std::optional{true} :
           s == "false" ? std::optional{false} : std::nullopt;
}

/// Parse a sequence of bytes, with optional space padding
static bool parse_bytes(std::string s, std::vector<uint8_t> & into)
{
    std::regex byte{"\\s*([0-9a-fA-F]{2})"};
    auto it = std::sregex_iterator{s.cbegin(), s.cend(), byte};
    decltype(it) last;
    for (; it != last; ++it)
    {
        if (it->size() != 2) break;
        auto b = parse<uint8_t>((*it)[1], 16);
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
static bool parse(std::string s, std::vector<uint8_t> & into)
{
    std::regex literal{"h'((?:[0-9a-fA-F]{2})*)'"};
    std::smatch m;
    if (std::regex_match(s.cbegin(), s.cend(), m, literal))
    {
        return parse_bytes(m[1], into);
    }
    return false;
}

/// Parse a list of integers
static bool parse(std::string s, std::vector<int> & into)
{
    std::regex literal{R"-((([0-9]+)(\s*,\s*[0-9]+)*)?)-"};
    std::smatch m;
    if (std::regex_match(s.cbegin(), s.cend(), m, literal))
    {
    }
    return false;
}

int main(int argc, char ** argv)
{
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
            std::cout << "Error in line " << line << "\n";
        }
        auto major = parse<uint8_t>(m[MajorNum]);
        auto minor = parse<uint8_t>(m[MinorNum]);
        if (!major || !minor)
        {
            std::cout << "Failed to parse initial byte "
                      << m[MajorNum] << ":" << m[MinorNum]
                      << " in " << line << "\n";
            continue;
        }
        std::vector<uint8_t> bytes;
        bytes.push_back( (*major<<5) | *minor );
        if (!parse_bytes(m[Bytes], bytes))
        {
            std::cout << "Failed to parse bytes "
                      << m[Bytes] << " in " << line << "\n";
            continue;
        }
        std::span encoded{bytes};

        // Dispatch the test on each type
#define DISPATCH(TYPE)                                                        \
        if (m[Type] == #TYPE)                                                 \
        {                                                                     \
            std::optional<TYPE> expected = parse<TYPE>(m[Decoded]);           \
            std::optional<TYPE> decoded = Cbor::Cbor<TYPE>::decode(encoded);  \
            if (!decoded || !expected || !eq(*decoded, *expected))            \
            {                                                                 \
                std::cerr << "Failed to decode " << m[Encoded] << "\tinto\t"  \
                          << *expected << "\tgot\t" << *decoded << "\n";      \
                continue;                                                     \
            }                                                                 \
            std::vector<uint8_t> buf(bytes.size(), 0);                        \
            std::span encoded{buf};                                           \
            auto ok = Cbor::Cbor<TYPE>::encode(*expected, encoded);           \
            if (!ok || !std::ranges::equal(bytes, buf))                       \
            {                                                                 \
                std::cerr << "Failed to encode " << m[Decoded] << "\tinto\t"; \
                for (auto c : bytes)                                          \
                    fprintf(stderr, "%02X", c);                               \
                fprintf(stderr, "\tgot\t");                                   \
                for (auto s = &*buf.begin(); s != &*encoded.begin(); ++s)     \
                    fprintf(stderr, "%02X", *s);                              \
                std::cerr << "\n";                                            \
            }                                                                 \
        }
        DISPATCH(uint64_t)
        else DISPATCH(int64_t)
        else DISPATCH(bool)
#if defined(CBOR_HALF_SUPPORT)
        else DISPATCH(_Float16)
#endif
        else DISPATCH(float)
        else DISPATCH(double)
        else if (m[Type] != "N/A" && m[Type].str().substr(0, 4) != "Skip")
        {
            std::cout << m[Type] << " | " << m[Decoded] << " | " << m[Encoded] << " |\n";
        }
    }
}
