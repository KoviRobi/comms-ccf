#pragma once

#include "cobs.hpp"

#include <stddef.h>
#include <stdint.h>

namespace Cobs
{
    class Encoder;
}

extern "C"
{
    Cobs::Encoder * cobsEncoderNew(const uint8_t * src, size_t srcLen);
    void cobsEncoderDelete(Cobs::Encoder * state);
    size_t cobsEncode(Cobs::Encoder * state, uint8_t * dest, size_t destLen);
}
