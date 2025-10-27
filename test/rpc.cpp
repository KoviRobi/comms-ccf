#include "ccf.hpp"
#include "rpc.hpp"

#include <stdint.h>
#include <stdio.h>
#include <termios.h>

#include <string_view>

using namespace std::literals;

// Just an illustration of where the semaphore/notification should be used
static bool notification = false;

static Ccf<{
    .rxBufSize = 256,
    .txBufSize = 256,
    .maxPktSize = 255,
}> ccf;

Rpc rpc
{
    Call{"add", "return x+y", {"x", "y"}, +[](int x, int y) { return x + y; }},
    Call{"hello", "greet the world", {}, +[](){ return "Hello, world!"sv; }},
};

static void rxIsr(uint8_t byte)
{
    if (ccf.receiveCharacter(byte))
    {
        notification = true;
    }
}

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
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);
    int i;
    while ( (i = fgetc(stdin)) != EOF)
    {
        uint8_t byte = static_cast<char>(i);
        rxIsr(byte);
        // Comms-CCF RPC thread
        if (notification)
        {
            notification = false;
            ccf.poll(rpc);
        }
        // Specific to this implementation: TX before getc to print
        // before blocking
        txIsr();
    }
}
