/**
\file
\brief Simple framing (marking packet start/stop) with
self-synchronisation.

# Constant Overhead Byte Stuffing (COBS)

If you have packets and you want to mark their end, you have two choices:

1. Put size at the start: this is not self-synchronizing, that is if you miss
   the start (or have an error in it) then you can't recover.
2. Delimit them, say with the zero byte. This is what we do.

However, the zero byte may appear in the packet, so you need to escape it
somehow. The simple solution of using an escape character (say '\', then the
ASCII digit '0' for zero, and '\' for an escaped escape) can end up doubling
the encoded data size in some cases.

Another approach is byte-stuffing: instead of using the zero byte, put an extra
byte at the start, with a value which is the count to the next escaped zero
byte. Then at that position you put the count to the next zero byte and so on.
If there is no zero byte, then you put the count to the end of the packet.

The remaining issue now is that a byte can only represent the range [0, 255] so
the value 0xFF doesn't represent that there is a zero after 255 bytes, but that
there are no zeros for 255 bytes, after which there is another extra byte.

This way the overhead is at most one extra byte every 255 bytes. And if there
are zeros more frequently than that, then the overhead can be just one extra
byte!

*/

#pragma once

#include <stddef.h>
#include <stdint.h>

#include <iterator>
#include <ranges>
#include <span>
#include <utility>

namespace Cobs
{
    /// Because we output `runLength + 1`, i.e. the pointer to the null
    /// not the last non-null byte, the maximum is 254
    constexpr uint8_t maxRunLength = 254;

    /// Returns the maximum size required for encoding data of the
    /// given size.
    constexpr size_t maxEncodedSize(size_t dataSize)
    {
        size_t overhead = (dataSize + maxRunLength)/maxRunLength;
        return dataSize + overhead;
    }

    /// Encodes data from a buffer (given at constructino time) to the
    /// output of the iterator.
    ///
    /// \note Encoder is itself the iterator.
    class Encoder
    {
    public:
        template<size_t Extent>
        Encoder(std::span<const uint8_t, Extent> _data)
        : data(_data), runLength(findRunLength()) { }

        template<size_t Extent>
        Encoder(std::span<uint8_t, Extent> span)
            : Encoder(std::span{
                reinterpret_cast<const uint8_t *>(span.data()),
                span.size()})
        {
        }

        Encoder begin()  const { return *this; }
        std::nullptr_t end() const { return {}; }

        // Iterator functionality
        using difference_type = ptrdiff_t;
        using value_type = uint8_t;

        value_type operator*() const;
        Encoder & operator++();
        Encoder operator++(int) { return ++*this; }
        bool operator!=(std::nullptr_t) const;

    private:
        /// Encoding by first finding the run-length, then outputting the
        /// header + run does loop over the data twice, but it allows us to
        /// have an iterator format rather than copying to a buffer (because we
        /// compute the header after the run) and then sending from the buffer.
        /// And that might end up looping over the data twice too, the second
        /// time in the copying from the buffer.
        ///
        /// Ultimately, not a problem until it is clear it becomes one.
        uint8_t findRunLength();

        std::span<const uint8_t> data;
        /// A run is a sequence of contiguous non-zeroes (excluding the zero).
        uint8_t runLength;
        uint8_t runIndex = 0;
        bool runHeaderOutput = false;
    };
    static_assert(std::input_iterator<Encoder>);

    struct enc
    {
        // Note: both iterator and range for now, to be able to do
        // comparisons for "inBegin != inEnd"
        template<class InBegin, class InEnd>
        struct iiter
        {
            using value_type = uint8_t;
            using difference_type = ptrdiff_t;

            InBegin inBegin;
            // TODO: Normally we would have this outside of iterator
            InEnd inEnd;
            /// A run is a sequence of contiguous non-zeroes (excluding the zero).
            bool runHeaderOutput = false;
            uint8_t runLength = 0;
            uint8_t runIndex = 0;
            uint8_t buf[maxRunLength];

            constexpr iiter() { }

            template<std::ranges::input_range R>
            constexpr iiter(R && r)
                : inBegin(std::cbegin(std::forward<R>(r))),
                  inEnd(std::cend(std::forward<R>(r)))
            {
                fill();
            }

            constexpr iiter begin() const { return *this; }
            constexpr iiter end() const { return *this; }

            constexpr value_type operator*() const
            {
                // return run header or current byte
                if (!runHeaderOutput)
                {
                    // Pointer to the first zero byte, not the last non-zero byte
                    return runLength + 1;
                }
                else
                {
                    return buf[runIndex];
                }
            }
            constexpr iiter & operator++()
            {
                // If not run header output, set it true
                // If at end, refill buffer
                if (!runHeaderOutput)
                {
                    runHeaderOutput = true;
                }
                else if (runIndex < runLength)
                {
                    ++runIndex;
                }
                // Not `else if`, need to recalculate before dereference
                if (runIndex == runLength)
                {
                    if (inBegin != inEnd)
                    {
                        if (runLength < maxRunLength)
                        {
                            // Skip over zero byte
                            ++inBegin;
                        }
                        runHeaderOutput = false;
                        fill();
                    }
                }
                return *this;
            }
            constexpr void fill()
            {
                for (
                    runIndex = 0, runLength = 0;
                    inBegin != inEnd &&
                        *inBegin != 0 &&
                        runLength < maxRunLength;
                    ++runLength, ++inBegin
                )
                {
                    buf[runLength] = *inBegin;
                }
            }
            constexpr iiter operator++(int)
            {
                const auto tmp  = *this;
                ++*this;
                return tmp;
            }
            constexpr bool operator!=(const iiter &) const
            {
                return !runHeaderOutput || runIndex < runLength || inBegin != inEnd;
            }
        };
        // static_assert(std::input_iterator<iiter<const uint8_t *, const uint8_t *>>);
        template<std::ranges::input_range R>
        iiter(R && r) -> iiter<decltype(std::cbegin(r)), decltype(std::cend(r))>;

        // struct irange
        // {
        //     template<class R>
        //     constexpr irange(R && range);
        //     constexpr iiter begin();
        //     constexpr iiter end();
        // };
        // static_assert(std::ranges::input_range<irange>);

        // Note: both iterator and range for now, to be able to do
        // comparisons for "inBegin != inEnd"
        // Note: Use as
        //     *oIter = <value>;
        //     oIter++
        // instead of
        //     *oIter++ = <value>;
        // for now
        // TODO: If we make iterator have a ref to the buffer/state then this could work
        template<class OutBegin, class OutEnd>
        struct oiter
        {
            using value_type = uint8_t;
            using difference_type = ptrdiff_t;

            OutBegin outBegin;
            OutEnd outEnd;

            /// A run is a sequence of contiguous non-zeroes (excluding the zero).
            bool runHeaderOutput = false;
            uint8_t runIndex = 0;
            uint8_t runLength = 0;
            uint8_t buf[maxRunLength];

            constexpr oiter() { }

            template<std::ranges::output_range<value_type> R>
            constexpr oiter(R && r)
                : outBegin(std::begin(std::forward<R>(r))),
                  outEnd(std::end(std::forward<R>(r)))
            {
            }

            constexpr value_type & operator*()
            {
                // return current buf reference
                return buf[runLength];
            }
            constexpr oiter & operator++()
            {
                // Just keep copying into the buffer until we hit a
                // delimiter or need another header
                if (buf[runLength] == 0)
                {
                    flush();
                }
                else
                {
                    if (runLength < maxRunLength)
                    {
                        ++runLength;
                    }
                    // Not else, we may now be at the end of the buffer
                    if (runLength == maxRunLength)
                    {
                        // Otherwise it's time to copy the whole buffered
                        // data to the output
                        flush();
                    }
                }
                return *this;
            }
            constexpr void flush()
            {
                if (outBegin != outEnd && !runHeaderOutput)
                {
                    *outBegin++ = runLength + 1;
                    runHeaderOutput= true;
                }
                for (
                    ;
                    outBegin != outEnd && runIndex < runLength && buf[runIndex] != 0;
                    ++runIndex
                )
                {
                    *outBegin++ = buf[runIndex];
                }
                if (runIndex == runLength || buf[runIndex] == 0)
                {
                    runLength = 0;
                    runIndex = 0;
                    runHeaderOutput = false;
                }
            }
            constexpr oiter operator++(int)
            {
                const auto tmp  = *this;
                ++*this;
                return tmp;
            }
            constexpr bool operator!=(const oiter &) const
            {
                return !runHeaderOutput || runIndex < runLength || outBegin != outEnd;
            }
        };
        static_assert(std::output_iterator<oiter<uint8_t *, uint8_t>, uint8_t>);
        template<std::ranges::output_range<uint8_t>  R>
        oiter(R && r) -> oiter<decltype(std::begin(r)), decltype(std::end(r))>;
    };

    /// Decodes data as a state-machine.
    class Decoder
    {
    public:
        /// Feeds a character to the state-machine, returning whether that
        /// character is to be emitted, or only causing an internal transition
        bool feed(uint8_t byte);

        /// Get the current character, should be called before feed, with the
        /// same byte that will be fed. But it is only valid if `feed` returns
        /// true, otherwise it should be ignored.
        uint8_t get(uint8_t byte) const;

    private:
        uint8_t runLength = 0;
        bool runLengthWasMax = true;
    };

    struct dec
    {
        struct iiter;
        struct iiter_pipe
        {
            template<class T>
            constexpr iiter operator|(T);
        };
        struct iiter
        {
            static constexpr iiter_pipe operator()();
            using value_type = uint8_t;
            using difference_type = ptrdiff_t;
            constexpr value_type operator*() const;
            constexpr iiter & operator++();
            constexpr iiter operator++(int);
        };
        static_assert(std::input_iterator<iiter>);

        struct oiter;
        struct oiter_pipe
        {
            template<class T>
            constexpr oiter operator>>(T);
        };
        struct oiter
        {
            static constexpr oiter_pipe operator()();
            using value_type = uint8_t;
            using difference_type = ptrdiff_t;
            constexpr value_type & operator*();
            constexpr oiter & operator++();
            constexpr oiter operator++(int);
        };
        static_assert(std::output_iterator<oiter, uint8_t>);
    };
};
