/**
\file
\brief Compile-time string manipulation.

# Compile-time string manipulation

We want to be able to create the RPC schema at compile time, for which
we need some helpers to do compile-time string manipulation.

More specifically, we manipulate compile-time arrays, which we can pass
using the copy constructor. And define a string-view operation on it.

*/

#include <stddef.h>

#include <algorithm>
#include <array>
#include <string_view>

template<size_t Size>
struct CompTimeString
{
    static constexpr size_t size = Size;

    operator std::string_view() const { return std::string_view{buf.data(), Size - 1}; }

    constexpr CompTimeString() { }
    constexpr CompTimeString(const char (&buf_)[Size])
    {
        for (size_t i = 0; i < Size; ++i)
        {
            buf[i] = buf_[i];
        }
    }
    constexpr CompTimeString(const std::array<char, Size> & buf_)
    {
        for (size_t i = 0; i < Size; ++i)
        {
            buf[i] = buf_[i];
        }
    }

    template<size_t Other>
    constexpr CompTimeString<Size - 1 + Other>
    operator+(const CompTimeString<Other> & other) const
    {
        CompTimeString<Size - 1 /* null */ + Other> ret{};
        auto it = std::data(ret.buf);
        it = std::copy_n(std::data(buf), Size - 1, it);
        it = std::copy_n(std::data(other.buf), Other - 1, it);
        ret.buf[decltype(ret)::size - 1] = 0;
        return ret;
    }

    template<size_t Other>
    constexpr CompTimeString<Size - 1 + Other + 2>
    operator,(const CompTimeString<Other> & other) const
    {
        CompTimeString<Size - 1 /* null */ + Other + 2 /* ", " */> ret{};
        auto it = std::data(ret.buf);
        it = std::copy_n(std::data(buf), Size - 1, it);
        it = std::copy_n(std::data(", "), 2, it);
        it = std::copy_n(std::data(other.buf), Other - 1, it);
        ret.buf[decltype(ret)::size - 1] = 0;
        return ret;
    }

    std::array<char, Size> buf;
};
