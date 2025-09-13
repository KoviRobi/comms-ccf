/**

# CCF -- Bringing It Together

This header brings the separate parts (COBS, CBOR, FNV-1A and the circular
buffer) together to be able to do RPC calls.

*/

#pragma once

#include "circular_buffer.hpp"
#include "cobs.hpp"
#include "fnv1a.hpp"

#if defined(DEBUG_CCF)
#include DEBUG_CCF
#else
#include "ndebug.hpp"
#endif

#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <iterator>
#include <optional>

struct CcfConfig
{
    size_t rxBufSize;
    size_t txBufSize;
    size_t maxPktSize;
};

template<CcfConfig Config>
class Ccf
{
    using RxNotification = CircularBuffer<uint8_t, Config.rxBufSize, Config.maxPktSize>::Notification;
public:
    using TxNotification = CircularBuffer<uint8_t, Config.txBufSize, Config.maxPktSize>::Notification;

    /// Safe to call from interrupt context
    ///
    /// Call this with any characters received on the transport. It
    /// returns `true` if it is time to call `poll`.
    bool receiveCharacter(uint8_t byte)
    {
        // Do get before feed, and always do feed to e.g. reset on \0
        const uint8_t value = decoder.get(byte);
        const bool do_output = decoder.feed(byte);
        // Not storing null byte because packet length is indicated in
        // a different way
        if (byte == 0)
        {
            if (!rxBuf.dropping())
            {
                rxBuf.notify();
            }
            else
            {
                rxBuf.reset_dropped();
            }
            return true;
        }
        else if (do_output)
        {
            rxBuf.push_back(value);
        }
        else
        {
            // This is expected for the header byte and continuation bytes
            // (frames >255 bytes without any null terminators)
            debugf(DEBUG "dropping %02X as !=0 and !do_output" END, START, byte);
        }
        return false;
    }

    /// Safe to call from interrupt context
    bool charactersToSend(std::optional<TxNotification> & notification)
    {
        return txBuf.get_notification(notification);
    }

    /// **Not safe** to call from an interrupt context
    ///
    /// Use an external synchronisation mechanism to call this after a
    /// null byte has been received. (It just won't have anything to do
    /// until then.)
    template<typename Rpc>
    void poll(const Rpc & rpc)
    {
        std::optional<RxNotification> notification;
        while (rxBuf.get_notification(notification))
        {
            size_t len = 0;
            // Copy to a local buffer to make sure it is contiguous
            for (auto c : *notification)
            {
                pktBuf[len++] = c;
            }
            std::span span{pktBuf, len};

            if (len < 6)
            {
                // TODO: Just using checksumless zero-length packets to
                // indicate error for now.
                debugf(WARN "Bad RPC! (len=%zu)" END, START, len);
                std::ranges::copy("Bad RPC!\n", std::back_inserter(txBuf));
                txBuf.push_back(static_cast<uint8_t>(0));
                txBuf.notify();
                continue;
            }

            auto channel = span[0];
            // TODO: Dispatch on channel
            (void)channel;

            if (!Fnv1a::checkAtEnd(span))
            {
                // TODO: Just using checksumless zero-length packets to
                // indicate error for now.
                debugf(WARN "Corrupted request (chan=%u)" END, START, channel);
                std::ranges::copy("Corrupted request\n", std::back_inserter(txBuf));
                txBuf.push_back(static_cast<uint8_t>(0));
                txBuf.notify();
                continue;
            }
            // Remove channel + checksum
            span = span.subspan(1, span.size() - 4 - 1);

            auto function = span[0];
            span = span.subspan(1);
            // Reuse the pktBuf buffer for return (leaving space for
            // channel, function, and checksum)
            pktBuf[0] = channel;
            pktBuf[1] = function;
            auto ret = std::span<uint8_t>(pktBuf + 2, sizeof(pktBuf) - 2 - 4);
            if (!rpc.call(function, span, ret))
            {
                // TODO: Just using checksumless zero-length packets to
                // indicate error for now.
                debugf(WARN "RPC failed (function=%u)" END, START, function);
                std::ranges::copy("RPC failed\n", std::back_inserter(txBuf));
                txBuf.push_back(static_cast<uint8_t>(0));
                txBuf.notify();
                continue;
            }

            // +4 for the checksum, the start bytes are accounted for
            auto respLen = static_cast<size_t>(ret.data() - pktBuf) + 4;
            std::span resp{pktBuf, respLen};
            Fnv1a::putAtEnd(resp);
            for (auto c : Cobs::Encoder(resp))
            {
                txBuf.push_back(c);
            }
            txBuf.push_back(static_cast<uint8_t>(0));
            txBuf.notify();
        }
    }

private:
    CircularBuffer<uint8_t, Config.txBufSize, Config.maxPktSize> txBuf;
    CircularBuffer<uint8_t, Config.rxBufSize, Config.maxPktSize> rxBuf;
    Cobs::Decoder decoder{};
    uint8_t pktBuf[Config.maxPktSize];
};

// This is a header, undefine the debugf macro
#undef debugf
