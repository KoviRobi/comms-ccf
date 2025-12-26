/**
\file
\brief Buffer for queuing received bytes.

# Circular buffer

As data arrives in the interrupt of the driver, it needs to be queued
for an application thread to process it. This class is a simple circular
buffer that can support either bytes for a pre-allocated buffer, or
pointers for a dynamically sized buffer.

Though because it includes the size of the data, it is optimised for
storing data not pointers -- as it would just waste half of the space for
pointers as size of a pointer is known. This is done because even though
we know the total readable size of the buffer, the buffer might contain
multiple packets, and we want to know where each packet starts and ends.

The basis of the circular buffer is based on the Lamport Queue, the basic
Lamport Queue uses two cursors, one for read and one for write, and a
buffer which is a size of power of two (see XXX). It is a concurrent,
single producer single consumer queue, which means there are some subtle
considerations:

1. the queue may be pushed to and popped from concurrently (though not
   pushed by multiple different threads concurrently, or popped from
   concurrently);
2. because the queue size is a power of two, the difference between the
   write and read cursors is always valid even if one or both of them
   have wrapped. Strictly speaking the requirement is that the queue
   size and the cursors wrap to zero at the same time, but it is easiest
   to acheive by having the cursors wrap naturally at a power of two,
   and having the queue size a power of two is probably not a huge
   restriction.

The modification we have done to the standard Lamport Queue is having
another cursor which is always bounded by the read and write cursors,
and is used to keep track of where the next packet is which we haven't
notified the consumer of. This allows us to store the length of the
packet at the start of the packet, and also to drop packets which cannot
fit into the buffer.

This does mean that we need to know how big a packet can be (specifically,
how many bytes to allocate for the packet size pointer), but this is
fine for now.

\todo Currently this was written for architectures which don't reorder
reads/writes, in my case small microcontrollers without caches and write
buffers, but needs to be fixed by using atomics on the cursors and the
buffer to ensure the cursors point to valid data (by the time the updated
cursor is read, the value at that location has been written).

\todo This does not expose an interface for DMA engines, but this could
fairly easily be added by getting the current next contiguous unused
part of the buffer (i.e. the part before any wrapping) and passing that
to the DMA engine.

\todo Forward and bidirectional iterator/range for Iterator/Frame.

*/

#pragma once

#include "types.hpp"

#if defined(DEBUG_CIRC_BUF)
#include DEBUG_CIRC_BUF
#else
#include "ndebug.hpp"
#endif

#include <stddef.h>
#include <stdint.h>

#include <array>
#include <bit>
#include <optional>
#include <type_traits>
#include <utility>

template<typename Value, size_t Size, size_t MaxPacketSize>
class CircularBuffer
{
public:
    /// The wraparound of the index needs to be divisible by the size
    /// of the container, so that when index wraps to 0, it also maps to
    /// `index % Size` to `0`.
    static_assert(std::popcount(Size) == 1, "Size needs to be a power of 2");

    using value_type = Value;

    static constexpr size_t sizeBytes = sizeof(SmallestTypeT<MaxPacketSize>);

    constexpr size_t capacity() const { return Size; }
    size_t size() const { return write - read; }
    size_t readable() const { return notified - read; }
    size_t unnotified() const { return std::max(sizeBytes, write - notified) - sizeBytes; }
    bool empty() const { return size() == 0; }
    bool full() const { return size() == capacity(); }
    bool dropping() const { return dropped; }

    /// Put an element onto the back of the queue. Using perfect
    /// forwarding to work for `const Value &` and also `Value &&`
    /// references.
    template<typename Value_>
        // Here to avoid an extra copy of the code with int for e.g. uint8_t
        requires std::same_as<std::remove_cvref_t<Value>, std::remove_cvref_t<Value_>>
    void push_back(Value_ && v)
    {
        if (unnotified() >= MaxPacketSize || size() >= capacity())
        {
            dropped = true;
        }
        if ((!dropped) && (notified == write))
        {
            write += sizeBytes;
        }
        if (unnotified() >= MaxPacketSize || size() >= capacity())
        {
            dropped = true;
        }
        if (!dropped)
        {
            buf[write++ % Size] = std::forward<Value_>(v);
        }
    }

    /// Drop the first element from the queue.
    void pop_front()
    {
        if (read < notified)
        {
            ++read;
        }
    }

    /// Return the element at the front of the queue. Does not check
    /// for overflows.
    Value & front()
    {
        return buf[read % Size];
    }

    /// Return the element at the front of the queue. Does not check
    /// for overflows.
    const Value & front() const
    {
        return buf[read % Size];
    }

    /// Discards the partial packet and sets the buffer back to receive
    /// mode. You will want to call this when you have a frame delimiter
    /// and know you are about to receive a full packet.
    void reset_dropped()
    {
        if (dropped)
        {
            dropped = false;
            write = notified;
        }
    }

    /// Delimit the elements pushed since the most recent frame as one
    /// packet. The consumer can then use get_frame to get a range onto
    /// the queue.
    /// Note: you may want to only notify if `dropping()` is false,
    /// otherwise you are notifying of a know partial packet. If
    /// `dropping()`, then `reset_dropped()` will drop the partial packet.
    void notify()
    {
        // Since we are about to notify of a partial packet, we want to
        // call `reset_dropped` to make sure it is treated as a full
        // packet.
        reset_dropped();
        size_t size = unnotified();
        for (size_t i = sizeBytes; i > 0; --i)
        {
            buf[notified++ % Size] = static_cast<uint8_t>(size);
            size >>= 8;
        }
        notified = write;
    }

    class Frame;
    class Iterator
    {
    public:
        using value_type = Value;
        using difference_type = ptrdiff_t;

        Iterator(const CircularBuffer * parent_, size_t index_)
          : parent(parent_), index(index_) {}
        Iterator & operator++() { ++index; return *this; }
        Iterator operator++(int) { const auto tmp = *this; ++*this; return tmp; }
        const Value & operator*() const { return parent->buf[index % Size]; }
        bool operator!=(const Iterator & other) const
        {
            return parent != other.parent || index != other.index;
        }
        bool operator==(const Iterator & other) const { return !(*this != other); }

    private:
        friend class Frame;
        const CircularBuffer * parent;
        size_t index;
    };

    class Frame
    {
    public:
        Frame(CircularBuffer * parent_, size_t start, uint8_t len)
          : parent(parent_),
            begin_(parent, start),
            end_(parent, start + len) {}
        Frame(const Frame &) = delete;
        Frame & operator=(const Frame &) = delete;
        Frame(Frame && o) : begin_(o.begin_), end_(o.end_)
        {
            std::swap(parent, o.parent);
        }
        Frame & operator=(Frame && o)
        {
            parent = o.parent;
            o.parent = nullptr;
            begin_ = o.begin_;
            end_ = o.end_;
            return *this;
        }

        ~Frame()
        {
            if (parent)
            {
                parent->read = end_.index;
            }
        }

        Iterator & begin() { return begin_; }
        Iterator & end() { return end_; }

        CircularBuffer * parent = nullptr;
        Iterator begin_;
        Iterator end_;
    };

    /// Gets the oldest frame, dropping any current frame
    /// in the given optional (for FIFO behaviour).
    bool get_frame(std::optional<Frame> & frame)
    {
        // Ensure we drop the old one first, before reading from the queue
        frame.reset();
        if (dropped)
        {
            debugf(DEBUG "No packet as truncating" END LOGLEVEL_ARGS);
            return false;
        }
        /// Read once
        auto start = read;
        const auto end = notified;
        if (start == end)
        {
            // No next packet, compare with the next two `start != end`
            return false;
        }
        size_t size = 0;
        for (size_t i = sizeBytes; i > 0; --i)
        {
            if (start != end)
            {
                size <<= 8;
                size |= buf[start++ % Size];
            }
            else
            {
                // A bad state, and we checked for truncating above
                debugf(ERROR "Truncated next packet size" END);
            }
        }
        if (start != end)
        {
            frame.emplace(this, start, size);
        }
        else
        {
            // Almost certainly an error, we checked for truncating above
            debugf(ERROR "Zero length item" END LOGLEVEL_ARGS);
        }
        return frame.has_value();
    }

    /// Reset the queue to the initial, empty state.
    void reset()
    {
        read = 0;
        notified = 0;
        write = 0;
        dropped = false;
    }

private:
    std::array<Value, Size> buf;
    size_t read = 0;
    size_t notified = 0;
    size_t write = 0;
    bool dropped = false;
};

// This is a header, undefine the debugf macro
#include "debug_end.hpp"
