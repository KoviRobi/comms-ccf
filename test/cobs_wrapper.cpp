#include "cobs_wrapper.hpp"

#include "cobs.hpp"

Cobs::Encoder * cobsEncoderNew(const uint8_t * src, size_t srcLen)
{
    return new Cobs::Encoder(std::span{src, srcLen});
}

void cobsEncoderDelete(Cobs::Encoder * state)
{
    delete state;
}

size_t cobsEncode(Cobs::Encoder * state, uint8_t * dest, size_t destLen)
{
    size_t index = 0;
    for (const auto byte : *state)
    {
        if (index == destLen)
        {
            break;
        }
        dest[index++] = byte;
    }
    return index;
}
