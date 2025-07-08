#include "fnv1a_wrapper.hpp"

#include "fnv1a.hpp"

#include <stddef.h>
#include <stdint.h>
#include <type_traits>

uint32_t fnv1aHash(const uint8_t * buf, size_t len)
{
    return Fnv1a::checksum(std::span{std::remove_const_t<uint8_t *>(buf), len});
}
