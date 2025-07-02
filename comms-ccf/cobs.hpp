/**

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

#include <span>
#include <iterator>

namespace Cobs
{
    constexpr uint8_t maxRunLength = 254;

    constexpr size_t maxEncodedSize(size_t dataSize)
    {
        size_t overhead = (dataSize + maxRunLength)/maxRunLength;
        return dataSize + overhead;
    }

    class IteratorEnd { };

    /// Note: Encoder is itself the iterator
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
        IteratorEnd end() const { return {}; }

        // Iterator functionality
        using difference_type = ptrdiff_t;
        using value_type = uint8_t;

        value_type operator*() const;
        Encoder & operator++();
        Encoder operator++(int) { return ++*this; }
        bool operator!=(IteratorEnd) const;

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
        // A run is a sequence of contiguous non-zeroes (excluding the zero)
        uint8_t runLength;
        uint8_t runIndex = 0;
        bool runHeaderOutput = false;
    };
    static_assert(std::input_iterator<Encoder>);
};
