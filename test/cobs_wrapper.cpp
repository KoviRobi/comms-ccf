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

Cobs::Decoder * cobsDecoderNew()
{
    return new Cobs::Decoder();
}

void cobsDecoderDelete(Cobs::Decoder * state)
{
    delete state;
}

size_t cobsDecode(
    Cobs::Decoder * state,
    const uint8_t * data,
    size_t dataLen,
    uint8_t * output,
    size_t outputLen)
{
    size_t index = 0;
    for (size_t in = 0; in < dataLen; ++in)
    {
        if (data[in] == 0 || index == outputLen)
        {
            return index;
        }
        const uint8_t byte = state->get(data[in]);
        if (state->feed(data[in]))
        {
            output[index++] = byte;
        }
    }
    return index;
}
