/**

# CCF -- Bringing It Together

This header brings the separate parts (COBS, CBOR, FNV-1A and the circular
buffer) together to be able to do RPC calls.

*/

#pragma once

#include "circular_buffer.hpp"
#include "cbor.hpp"
#include "cobs.hpp"
#include "fnv1a.hpp"

#if defined(DEBUG_CCF)
#include DEBUG_CCF
#else
#include "ndebug.hpp"
#endif

#include <stdarg.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#include <algorithm>
#include <iterator>
#include <optional>
#include <span>

struct CcfConfig
{
    size_t rxBufSize;
    size_t txBufSize;
    size_t maxPktSize;
};

enum class Channels : uint8_t
{
    Rpc = 0,
    Log = 1,
    // TODO: In-place trace tag to save bytes on trace data?
    Trace = 2,
    // TODO: Reserve some bits for flags? E.g. fragment/partial packet
    // flag, CCF metadata/error flag?
};

enum class LogLevel : uint8_t
{
    Debug,
    Info,
    Warn,
    Error,

};

template<CcfConfig Config>
class Ccf
{
    using RxFrame = CircularBuffer<uint8_t, Config.rxBufSize, Config.maxPktSize>::Frame;
public:
    using TxFrame = CircularBuffer<uint8_t, Config.txBufSize, Config.maxPktSize>::Frame;

    /// Safe to call from interrupt context
    /// **Not threadsafe** only call from a single communications
    /// channel's interrupt callback. If you want to use multiple
    /// communications channels multiplexed, you could use separate SPSC
    /// queues feeding into this queue.
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
            debugf(DEBUG "dropping %02X as !=0 and !do_output" END LOGLEVEL_ARGS, byte);
        }
        return false;
    }

    /// Safe to call from interrupt context
    bool charactersToSend(std::optional<TxFrame> & frame)
    {
        return txBuf.get_frame(frame);
    }

    /// **Not threadsafe** only call from one thread
    /// **Not safe to call from an interrupt context**
    ///
    /// Process any incoming packets. Returns true if there is an output
    /// in response.
    ///
    /// Use an external synchronisation mechanism to call this after a
    /// null byte has been received. (It just won't have anything to do
    /// until then.)
    template<typename Rpc>
    bool poll(const Rpc & rpc)
    {
        bool output = false;
        std::optional<RxFrame> frame;
        while (rxBuf.get_frame(frame))
        {
            size_t len = 0;
            // Copy to a local buffer to make sure it is contiguous
            for (auto c : *frame)
            {
                pktBuf[len++] = c;
            }
            std::span span{pktBuf, len};

            if (len < 6)
            {
                // TODO: Just using checksumless zero-length packets to
                // indicate error for now.
                debugf(WARN "Bad RPC! (len=%zu)" END LOGLEVEL_ARGS, len);
                std::ranges::copy("Bad RPC!\n", std::back_inserter(txBuf));
                txBuf.push_back(static_cast<uint8_t>(0));
                txBuf.notify();
                output = true;
                continue;
            }

            uint8_t channel = span[0];
            // TODO: Dispatch on channel
            (void)channel;

            if (!Fnv1a::checkAtEnd(span))
            {
                // TODO: Just using checksumless zero-length packets to
                // indicate error for now.
                debugf(WARN "Corrupted request (chan=%u)" END LOGLEVEL_ARGS, channel);
                std::ranges::copy("Corrupted request\n", std::back_inserter(txBuf));
                txBuf.push_back(static_cast<uint8_t>(0));
                txBuf.notify();
                output = true;
                continue;
            }
            // Remove channel + checksum
            span = span.subspan(
                sizeof(channel),
                span.size() - Fnv1a::size - sizeof(channel));

            uint8_t seqNo = span[0];
            uint8_t function = span[1];
            span = span.subspan(sizeof(seqNo) + sizeof(function));
            // Reuse the pktBuf buffer for return (leaving space for
            // channel, function, and checksum)
            auto header = sizeof(channel) + sizeof(seqNo) + sizeof(function);
            auto ret = std::span<uint8_t>(
                pktBuf + header, sizeof(pktBuf) - header - Fnv1a::size);
            if (!rpc.call(function, span, ret))
            {
                // TODO: Just using checksumless zero-length packets to
                // indicate error for now.
                debugf(WARN "RPC failed (function=%u)" END LOGLEVEL_ARGS, function);
                std::ranges::copy("RPC failed\n", std::back_inserter(txBuf));
                txBuf.push_back(static_cast<uint8_t>(0));
                txBuf.notify();
                output = true;
                continue;
            }
            pktBuf[0] = channel;
            pktBuf[1] = seqNo;
            pktBuf[2] = function;
            auto respLen = static_cast<size_t>(ret.data() - pktBuf);
            std::span resp{pktBuf + sizeof(channel), respLen - sizeof(channel)};
            output = output || send(Channels::Rpc, resp);
        }
        return output;
    }

    /// **Not threadsafe** use a mutex
    ///
    /// Put some messaget to be sent, returns true if it has succeeded
    /// (and therefore you should call the function to send messages
    /// from the queue).
    ///
    /// TODO: Maybe could be re-entrant/support adding from ISR without
    /// locking by exchanging the ISR data buffer with the data not yet
    /// notified, then putting the data not yet notified back after. This
    /// would make it only not threadsafe for normal tasks which might
    /// get switched out between each-other
    bool send(Channels channel, std::span<uint8_t> & data)
    {
        const size_t toSend = data.size() + sizeof(channel) + Fnv1a::size;
        if (toSend > sizeof(pktBuf))
        {
            debugf("Data for send too large\n");
            return false;
        }
        std::span resp{pktBuf, toSend};
        // Note: memmove caters for overlapping pktBuf/data (though no
        // need to move if it is already in place)
        if (&data[0] != &resp[1])
        {
            memmove(&resp[1], data.data(), data.size_bytes());
        }
        uint8_t chan = static_cast<uint8_t>(channel);
        resp[0] = chan;
        Fnv1a::putAtEnd(resp);
        for (auto c : Cobs::Encoder(resp))
        {
            txBuf.push_back(c);
        }
        txBuf.push_back(static_cast<uint8_t>(0));
        if (txBuf.dropping())
        {
            txBuf.reset_dropped();
            return false;
        }
        else
        {
            txBuf.notify();
            return true;
        }
    }

    /// **Threadsafe** because it doesn't send the message, just formats
    /// it to the buffer.
    ///
    /// Log some data to the buffer, but don't send it yet. Handy for
    /// using in interrupts or other reentrant contexts (e.g. using it to
    /// log from inside an RPC call). Returns the number of bytes written
    /// to the buffer (all the bytes not just the formatted string),
    /// or nullopt if it failed to log.
    ///
    /// Once it is safe, call `send(Channels::Log, span & dataInBuf)`
    /// on the formatted data. This function changes the `span` to point
    /// to the unused region of the buffer, once done (if it successfully
    /// logged, otherwise it leaves it in place).
    ///
    /// TODO: Optionally just send the format string pointer (if it is
    /// in .rodata) and the client can read it from the ELF file (or
    /// download it separately)
#if defined(DEFERRED_FORMATTING)
    template <typename... Args>
    std::optional<size_t> logToBuffer(
        std::span<uint8_t> & span,
        LogLevel level,
        uint8_t module,
        std::string_view fmt,
        Args && ... args)
#else
    std::optional<size_t> logToBuffer(
        std::span<uint8_t> & span,
        LogLevel level,
        uint8_t module,
        const char * fmt,
        ...)
    {
        va_list args;
        va_start(args, fmt);
        const auto ret = vLogToBuffer(span, level, module, fmt, args);
        va_end(args);
        return ret;
    }
    /// Same as logToBuffer but taking `va_list` instead of `...` varargs.
    std::optional<size_t> vLogToBuffer(
        std::span<uint8_t> & span,
        LogLevel level,
        uint8_t module,
        const char * fmt,
        va_list args)
#endif
    {
        if (module > (1 << 5) - 1)
        {
            return {};
        }
        uint8_t initialByte = (static_cast<uint8_t>(level) << 5) | module;
        span[0] = initialByte;
        // Space for length
        const auto start = span.begin() + 2;
#if defined(DEFERRED_FORMATTING)
        const auto written = std::distance(start, std::ranges::copy(fmt, start).out);
        auto rest = span.subspan(2 + written);
        bool ok = (Cbor::Cbor<Args>::encode(std::forward<Args>(args), rest) && ...);
        const auto end = rest.begin();
#else
        int written = vsnprintf(
            reinterpret_cast<char *>(&*start),
            std::distance(start, span.end()),
            fmt,
            args
        );
        bool ok = 0 < written && static_cast<size_t>(written) < span.size();
        const auto end = start + written;
#endif
        span[1] = written;
        if (!ok)
        {
            return {};
        }
        size_t total = std::distance(span.begin(), end);
        span = span.subspan(total);
        return {total};
    }

    /// **Not threadsafe** use a mutex -- in particular don't call from
    /// inside RPC functions either!
    ///
    /// Log some data. Returns the number of bytes logged (all the bytes
    /// not just the formatted string), or nullopt if failed to log.
    ///
    /// TODO: Optionally just send the format string pointer (if it is
    /// in .rodata) and the client can read it from the ELF file (or
    /// download it separately)
#if defined(DEFERRED_FORMATTING)
    template <typename... Args>
    std::optional<size_t> log(
        LogLevel level,
        uint8_t module,
        std::string_view fmt,
        Args && ... args)
#else
    std::optional<size_t> log(
        LogLevel level,
        uint8_t module,
        const char * fmt,
        ...)
#endif
    {
        std::span<uint8_t> span{pktBuf};
#if defined(DEFERRED_FORMATTING)
        const auto formatted = logToBuffer(span, level, module, fmt, std::forward<Args>(args)...);
#else
        va_list args;
        va_start(args, fmt);
        const auto formatted = vLogToBuffer(span, level, module, fmt, args);
        va_end(args);
#endif
        if (formatted)
        {
            std::span<uint8_t> toSend{pktBuf, *formatted};
            if (send(Channels::Log, toSend))
            {
                return formatted;
            }
        }
        return {};
    }


private:
    CircularBuffer<uint8_t, Config.txBufSize, Config.maxPktSize> txBuf;
    CircularBuffer<uint8_t, Config.rxBufSize, Config.maxPktSize> rxBuf;
    Cobs::Decoder decoder{};
    uint8_t pktBuf[Config.maxPktSize];
};

// This is a header, undefine the debugf macro
#include "debug_end.hpp"
