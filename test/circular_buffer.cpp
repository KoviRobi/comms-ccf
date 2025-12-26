/**
\file

Runs some checks on the circular_buffer.

*/
#include "circular_buffer.hpp"

#include "test_utils.hpp"

#include <initializer_list>
#include <optional>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#include <algorithm>
#include <iterator>

using u8s = std::initializer_list<uint8_t>;

constexpr size_t MAX_PKT_SIZE = 4;
constexpr size_t BUF_SIZE = 8;
CircularBuffer<uint8_t, BUF_SIZE, MAX_PKT_SIZE> buf;
std::optional<decltype(buf)::Frame> frame;
std::back_insert_iterator ins{buf};

/// \test
/// Ensure that inserting a too large packet into the queue is handled
/// safely (dropped).
static bool test_insert_more_than_max_pkt_size()
{
    frame.reset();
    buf.reset();

    assert(buf.empty());
    assert(!buf.dropping());
    std::ranges::copy(u8s{1, 2, 3, 4, 5, 6, 7, 8}, ins);
    assert(buf.dropping());
    assert(buf.size() == MAX_PKT_SIZE + buf.sizeBytes);
    buf.reset_dropped();
    assert(buf.empty());
    return true;
}

/// \test
/// Tests filling the queue with maximum sized packets.
static bool test_fill_queue_max_pkt_size()
{
    frame.reset();
    buf.reset();

    assert(buf.empty());
    assert(!buf.dropping());

    std::ranges::copy(u8s{1, 2, 3}, ins);
    assert(buf.size() == 3 + buf.sizeBytes);
    assert(!buf.dropping());
    buf.notify();

    std::ranges::copy(u8s{4, 5, 6}, ins);
    assert(buf.size() == 2 * (3 + buf.sizeBytes));
    assert(!buf.dropping());
    buf.notify();

    assert(buf.full());
    assert(buf.get_frame(frame));
    assert(!buf.dropping());
    std::ranges::copy(u8s{7}, ins);
    assert(buf.dropping());
    return true;
}

/// \test
/// Tests filling the queue with small packets.
static bool test_fill_queue_small_pkts()
{
    frame.reset();
    buf.reset();

    assert(buf.empty());
    assert(!buf.dropping());

    std::ranges::copy(u8s{1}, ins);
    assert(buf.size() == 1 + buf.sizeBytes);
    assert(!buf.dropping());
    buf.notify();

    std::ranges::copy(u8s{2}, ins);
    assert(buf.size() == 2 * (1 + buf.sizeBytes));
    assert(!buf.dropping());
    buf.notify();

    std::ranges::copy(u8s{3}, ins);
    assert(buf.size() == 3 * (1 + buf.sizeBytes));
    assert(!buf.dropping());
    buf.notify();

    std::ranges::copy(u8s{4}, ins);
    assert(buf.size() == 4 * (1 + buf.sizeBytes));
    assert(!buf.dropping());
    buf.notify();

    assert(buf.full());
    assert(buf.get_frame(frame));
    assert(!buf.dropping());
    std::ranges::copy(u8s{5}, ins);
    assert(buf.dropping());
    return true;
}

static bool test_normal_operation()
{
    frame.reset();
    buf.reset();

    assert(buf.empty());
    assert(!buf.dropping());

    u8s p1{1, 2, 3};
    std::ranges::copy(p1, ins);
    assert(buf.size() == 3 + buf.sizeBytes);
    assert(!buf.dropping());
    buf.notify();

    u8s p2{4};
    std::ranges::copy(p2, ins);
    assert(buf.size() == 3 + 1 + 2 * buf.sizeBytes);
    assert(!buf.dropping());
    buf.notify();

    u8s p3{5};
    std::ranges::copy(p3, ins);
    assert(buf.full());
    assert(!buf.dropping());
    buf.notify();

    assert(buf.get_frame(frame));
    assert(std::equal(p1.begin(), p1.end(), frame->begin(), frame->end()));
    frame.reset();

    u8s p4{6, 7, 8};
    std::ranges::copy(p4, ins);
    assert(buf.full());
    assert(!buf.dropping());
    buf.notify();

    assert(buf.get_frame(frame));
    assert(std::equal(p2.begin(), p2.end(), frame->begin(), frame->end()));
    frame.reset();

    assert(buf.get_frame(frame));
    assert(std::equal(p3.begin(), p3.end(), frame->begin(), frame->end()));
    frame.reset();

    assert(buf.get_frame(frame));
    assert(std::equal(p4.begin(), p4.end(), frame->begin(), frame->end()));
    frame.reset();

    assert(buf.empty());
    assert(!buf.get_frame(frame));
    assert(!buf.dropping());
    return true;
}

int main()
{
    if (
        test_insert_more_than_max_pkt_size() &&
        test_fill_queue_max_pkt_size() &&
        test_fill_queue_small_pkts() &&
        test_normal_operation()
    ) {
        return 0;
    }
}
