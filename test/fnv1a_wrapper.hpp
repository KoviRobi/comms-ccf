#pragma once

#include <cstdint>
#include <stddef.h>
#include <stdint.h>

extern "C"
{
    uint32_t fnv1aHash(const uint8_t * data, size_t len);
}
