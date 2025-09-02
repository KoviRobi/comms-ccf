#include <ctype.h>
#include <stdint.h>
#include <stdio.h>

#include <span>

template<size_t Extent>
void hexdump(std::span<const uint8_t, Extent> data)
{
    for (size_t line = 0; line < data.size(); line += 16)
    {
        printf("%04lX: ", line);
        for (uint8_t col = 0; col < 16; col++)
        {
            if ( col == 0) { /* already printed */ }
            else if ((col & 7) == 0) { printf("    "); }
            else if ((col & 3) == 0) { printf("  ");   }
            else if ((col & 1) == 0) { printf(" ");   }
            size_t pos = line + col;
            if (pos < data.size())
            {
                printf("%02X", data[pos]);
            }
            else
            {
                printf("__");
            }
        }
        printf("    ");
        for (size_t col = 0; col < 16; col++)
        {
            size_t pos = line + col;
            if (pos < data.size())
            {
                if (isprint(data[pos]))
                {
                    printf("%c", data[pos]);
                }
                else
                {
                    printf(".");
                }
            }
            else
            {
                break;
            }
        }
        printf("\n");
    }
}

template<size_t Extent>
void hexdump(std::span<uint8_t, Extent> data)
{
    hexdump<Extent>(std::span<const uint8_t, Extent>{
        reinterpret_cast<const uint8_t *>(data.data()), data.size()});
}
