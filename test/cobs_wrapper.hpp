#pragma once

#include "cobs.hpp"

#include <span>
#include <stddef.h>
#include <stdint.h>

namespace Cobs
{
    class Encoder;
    class Decoder;
}

// #define IN
#if defined(IN)
using Iter = std::span<const uint8_t>::const_iterator;
using CobsEncoder = Cobs::enc::iiter<Iter, Iter>;
#else
using CobsEncoder = std::span<const uint8_t>;
#endif

extern "C"
{
    CobsEncoder * cobsEncoderNew(const uint8_t * src, size_t srcLen);
    void cobsEncoderDelete(CobsEncoder * state);
    size_t cobsEncode(CobsEncoder * state, uint8_t * dest, size_t destLen);

    Cobs::Decoder * cobsDecoderNew();
    void cobsDecoderDelete(Cobs::Decoder * state);
    size_t cobsDecode(
        Cobs::Decoder * state,
        const uint8_t * data,
        size_t dataLen,
        uint8_t * output,
        size_t outputLen);
}
