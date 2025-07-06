#pragma once

#include "cobs.hpp"

#include <stddef.h>
#include <stdint.h>

namespace Cobs
{
    class Encoder;
    class Decoder;
}

extern "C"
{
    Cobs::Encoder * cobsEncoderNew(const uint8_t * src, size_t srcLen);
    void cobsEncoderDelete(Cobs::Encoder * state);
    size_t cobsEncode(Cobs::Encoder * state, uint8_t * dest, size_t destLen);

    Cobs::Decoder * cobsDecoderNew();
    void cobsDecoderDelete(Cobs::Decoder * state);
    size_t cobsDecode(
        Cobs::Decoder * state,
        const uint8_t * data,
        size_t dataLen,
        uint8_t * output,
        size_t outputLen);
}
