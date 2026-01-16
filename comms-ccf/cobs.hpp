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
        // Note: both iterator and range for now, to be able to do
        // comparisons for "inBegin != inEnd"
        template<std::input_iterator InBegin, std::input_iterator  InEnd>
        struct Input
        {
            using value_type = uint8_t;
            using difference_type = ptrdiff_t;

            Encoder * enc;
            InBegin inBegin;
            // TODO: Normally we would have this outside of iterator
            InEnd inEnd;

            constexpr Input() { }

            template<std::input_iterator InBegin_, std::input_iterator InEnd_>
            constexpr Input(Encoder * enc_, InBegin_ && inBegin_, InEnd_ && inEnd_)
                : enc(enc_),
                  inBegin(std::forward<InBegin_>(inBegin_)),
                  inEnd(std::forward<InEnd_>(inEnd_))
            {
                fill();
            }

            constexpr Input begin() const { return *this; }
            constexpr Input end() const { return *this; }

            constexpr value_type operator*() const
            {
                // return run header or current byte
                if (!enc->runHeaderOutput)
                {
                    // Pointer to the first zero byte, not the last non-zero byte
                    return enc->runLength + 1;
                }
                else
                {
                    return enc->buf[enc->runIndex];
                }
            }
            constexpr Input & operator++()
            {
                // If not run header output, set it true
                // If at end, refill buffer
                if (!enc->runHeaderOutput)
                {
                    enc->runHeaderOutput = true;
                }
                else if (enc->runIndex < enc->runLength)
                {
                    ++enc->runIndex;
                }
                // Not `else if`, need to recalculate before dereference
                if (enc->runIndex == enc->runLength)
                {
                    if (inBegin != inEnd)
                    {
                        if (enc->runLength < maxRunLength)
                        {
                            // Skip over zero byte
                            ++inBegin;
                        }
                        enc->runHeaderOutput = false;
                        fill();
                    }
                }
                return *this;
            }
            constexpr void fill()
            {
                for (
                    enc->runIndex = 0, enc->runLength = 0;
                    inBegin != inEnd &&
                        *inBegin != 0 &&
                        enc->runLength < maxRunLength;
                    ++enc->runLength, ++inBegin
                )
                {
                    enc->buf[enc->runLength] = *inBegin;
                }
            }
            constexpr Input operator++(int)
            {
                const auto tmp  = *this;
                ++*this;
                return tmp;
            }
            constexpr bool operator!=(const Input &) const
            {
                return !enc->runHeaderOutput || enc->runIndex < enc->runLength || inBegin != inEnd;
            }
        };

        template<std::ranges::input_range R>
        Input(R && r) -> Input<decltype(std::cbegin(r)), decltype(std::cend(r))>;

        // static_assert(std::input_iterator<Input<const uint8_t *, const uint8_t *>>);
        // static_assert(std::ranges::input_range<Input<const uint8_t *, const uint8_t *>>);


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
        struct Output
        {
            using value_type = uint8_t;
            using difference_type = ptrdiff_t;

            Encoder * enc;
            OutBegin outBegin;
            // TODO: Normally we would have this outside of iterator
            OutEnd outEnd;

            constexpr Output() { }

            template<std::output_iterator<uint8_t> OutBegin_, std::output_iterator<uint8_t> OutEnd_>
            constexpr Output(Encoder * enc_, OutBegin_ && outBegin_, OutEnd_ && outEnd_)
                : enc(enc_),
                  outBegin(std::forward<OutBegin_>(outBegin_)),
                  outEnd(std::forward<OutEnd_>(outEnd_))
            {
            }

            constexpr value_type & operator*()
            {
                // return current buf reference
                return enc->buf[enc->runLength];
            }
            constexpr Output & operator++()
            {
                // Just keep copying into the buffer until we hit a
                // delimiter or need another header
                if (enc->buf[enc->runLength] == 0)
                {
                    flush();
                }
                else
                {
                    if (enc->runLength < maxRunLength)
                    {
                        ++enc->runLength;
                    }
                    // Not else, we may now be at the end of the buffer
                    if (enc->runLength == maxRunLength)
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
                if (outBegin != outEnd && !enc->runHeaderOutput)
                {
                    *outBegin++ = enc->runLength + 1;
                    enc->runHeaderOutput= true;
                }
                for (
                    ;
                    outBegin != outEnd && enc->runIndex < enc->runLength && enc->buf[enc->runIndex] != 0;
                    ++enc->runIndex
                )
                {
                    *outBegin++ = enc->buf[enc->runIndex];
                }
                if (enc->runIndex == enc->runLength || enc->buf[enc->runIndex] == 0)
                {
                    enc->runLength = 0;
                    enc->runIndex = 0;
                    enc->runHeaderOutput = false;
                }
            }
            constexpr Output operator++(int)
            {
                const auto tmp  = *this;
                ++*this;
                return tmp;
            }
            constexpr bool operator!=(const Output &) const
            {
                return !enc->runHeaderOutput || enc->runIndex < enc->runLength || outBegin != outEnd;
            }
        };

        template<std::ranges::output_range<uint8_t>  R>
        Output(R && r) -> Output<decltype(std::begin(r)), decltype(std::end(r))>;

        static_assert(std::output_iterator<Output<uint8_t *, uint8_t>, uint8_t>);

        template<class InBegin, class InEnd>
        Input<InBegin, InEnd> input(InBegin && inBegin, InEnd && inEnd)
        {
            return Input<InBegin, InEnd>(
                this,
                std::forward<InBegin>(inBegin),
                std::forward<InEnd>(inEnd)
            );
        }

        template<std::ranges::input_range R>
        Input<
            decltype(std::cbegin(std::forward<R>(std::declval<R>()))),
            decltype(std::cend(std::forward<R>(std::declval<R>())))
        > input(R && r)
        {
            return Input<
                decltype(std::cbegin(std::forward<R>(r))),
                decltype(std::cend(std::forward<R>(r)))
            >(
                this,
                std::forward<decltype(std::cbegin(std::forward<R>(r)))>(
                    std::cbegin(std::forward<R>(r))),
                std::forward<decltype(std::cend(std::forward<R>(r)))>(
                    std::cend(std::forward<R>(r)))
            );
        }

        template<std::output_iterator<uint8_t> OutBegin, std::output_iterator<uint8_t> OutEnd>
        Output<OutBegin, OutEnd> output(OutBegin && inBegin, OutEnd && inEnd)
        {
            return Output<OutBegin, OutEnd>(
                this,
                std::forward<OutBegin>(inBegin),
                std::forward<OutEnd>(inEnd)
            );
        }

        template<std::ranges::output_range<uint8_t> R>
        Output<
            decltype(std::begin(std::forward<R>(std::declval<R>()))),
            decltype(std::end(std::forward<R>(std::declval<R>())))
        > output(R && r)
        {
            return Output<
                decltype(std::begin(std::forward<R>(r))),
                decltype(std::end(std::forward<R>(r)))
            >(
                this,
                std::forward<decltype(std::begin(std::forward<R>(r)))>(
                    std::begin(std::forward<R>(r))),
                std::forward<decltype(std::end(std::forward<R>(r)))>(
                    std::end(std::forward<R>(r)))
            );
        }

    private:
        /// A run is a sequence of contiguous non-zeroes (excluding the zero).
        uint8_t runLength = 0;
        uint8_t runIndex = 0;
        bool runHeaderOutput = false;
        uint8_t buf[maxRunLength];

    };

    struct enc
    {
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
