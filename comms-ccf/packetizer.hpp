/**

# Packetizer

When you have a byte transport, you want to collect it into a larger unit,
let's call this a packet/frame. To do the framing we have the Constant Overhead
Byte-Stuffing (COBS), so all we need to do now is put a checksum at the end and
also have a queue into which we can receive bytes until we have a full packet.

*/

#pragma once

#include <stdint.h>

#include <optional>

namespace Packetizer
{
    class Send
    {
    public:
        /// This is a callback to call when the transport has finished sending
        /// a byte, to schedule sending the next byte. It will return
        /// std::nullopt when we have no more bytes to send.
        std::optional<uint8_t> getNextByte();

    private:
        /// This signals that we have bytes to send, can be ignored when
        /// sending is already in progress, the `getNextByte` will already
        /// contain the data when this is called
        bool (*sendByte)(uint8_t);
    };

    /// We want to encode something along the lines of
    /// ```
    /// using namespace std::ranges::views;
    /// for (auto & pkt :
    ///     UART_RX_ITERATOR |
    ///     split<0> |
    ///     transform(Cobs::Decode) |
    ///     transform(Buffer<MAX_PKT_SIZE>))
    /// {
    ///     if (fnv1a_checksum(pkt))
    ///     {
    ///         // Use packet
    ///     }
    /// }
    /// ```
    class Receive
    {
    public:
        bool receiveByte(uint8_t byte);

    private:
        /// This is the function we call when we have found a frame
    };
};
