#include "itertools.hpp"

#include <stdint.h>
#include <stdio.h>

#include <algorithm>
#include <array>

int main()
{
    auto in = std::to_array<uint8_t>({1,2,3,4,5,0,1,2,3,4,5,0, 1});
    std::array<uint8_t, in.size()> outBuf;
    auto out = Split<uint8_t, uint8_t *, uint8_t *>(0, outBuf);
    std::ranges::copy(in, out);

    printf("[");
    auto sep = "";
    for (uint8_t c : outBuf)
    {
        printf("%s%d", sep, c);
        sep = ", ";
    }
    printf("]\n");
}
