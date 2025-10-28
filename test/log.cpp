#include "ccf.hpp"

#include <stdint.h>
#include <stdio.h>
#include <termios.h>

#include <string_view>

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
    // Comms-CCF Log example thread
    {
        ccf.log(LogLevel::Debug, 0, "debug int %d", 0);
        ccf.log(LogLevel::Info, 1, "debug float %f", 1.0f);
        ccf.log(LogLevel::Info, 2, "debug str %s", "two");
        ccf.log(LogLevel::Info, 3, "debug str %s", (const char *)"three");
    }
    txIsr();
}
