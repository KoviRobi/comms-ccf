#include "ccf.hpp"

#include <stdint.h>
#include <stdio.h>
#include <termios.h>

#include <concepts>
#include <string_view>

using namespace std::string_view_literals;

static Ccf<{
    .rxBufSize = 256,
    .txBufSize = 256,
    .maxPktSize = 255,
}> ccf;

static void txIsr()
{
    std::optional<decltype(ccf)::TxFrame> toTx;
    while (ccf.charactersToSend(toTx))
    {
        for (auto c : *toTx)
        {
            fputc(c, stdout);
        }
    }
}

int main()
{
    setvbuf(stdout, NULL, _IONBF, 0);
    // Comms-CCF Plot example thread
    {
        ccf.sendCbor(Channels::Plot, "exp"sv, 0, 0);
        ccf.sendCbor(Channels::Plot, "exp"sv, 1, 1);
        ccf.sendCbor(Channels::Plot, "exp"sv, 2, 4);
        ccf.sendCbor(Channels::Plot, "exp"sv, 3, 8);
    }
    txIsr();
}
