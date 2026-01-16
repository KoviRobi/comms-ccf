#include "cobs_wrapper.hpp"

#include "cobs.hpp"

#include <algorithm>

#if defined(IN)
CobsEncoder * cobsEncoderNew(const uint8_t * src, size_t srcLen)
{
    return new CobsEncoder(std::span<const uint8_t>{src, srcLen});
}

void cobsEncoderDelete(CobsEncoder * state)
{
    delete state;
}

size_t cobsEncode(CobsEncoder * state, uint8_t * dest, size_t destLen)
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
#else
CobsEncoder * cobsEncoderNew(const uint8_t * src, size_t srcLen)
{
    return new std::span{src, srcLen};
}

void cobsEncoderDelete(CobsEncoder * state)
{
    delete state;
}

size_t cobsEncode(CobsEncoder * state, uint8_t * dest, size_t destLen)
{
    std::span span{dest, destLen};
    Cobs::Encoder encoder;
    auto out = encoder.output(span);
    auto in_out = std::ranges::copy(*state, out);
    auto in = in_out.in;
    out = in_out.out;
    out.flush();
    *state = state->subspan(in - state->begin());
    return out.outBegin - span.begin();
}
#endif

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
