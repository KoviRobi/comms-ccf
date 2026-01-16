#include "cobs.hpp"

#include "hexdump.hpp"

#include <stdint.h>
#include <stdio.h>

#include <array>
#include <span>

int main()
{
    std::array<uint8_t, 1> buf;
    size_t i = 0;
    for (auto & byte : buf)
    {
        byte = (i++ % 255) + 1;
    }
    std::array<uint8_t, Cobs::maxEncodedSize(buf.size())> enc;

    i = 0;
    Cobs::Encoder encoder{};
    for (auto byte : encoder.input(std::span{buf}))
    {
        if (i == enc.size())
        {
            break;
        }
        enc[i++] = byte;
    }

    hexdump(std::span{buf});
    int n = printf("Encoded %ld/%ld ", i, enc.size());
    for (n = 70 - n; n > 0; --n) { printf("="); }
    printf("\n");
    hexdump(std::span{enc});
}
