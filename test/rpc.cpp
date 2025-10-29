#include "ccf.hpp"
#include "rpc.hpp"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <termios.h>

#include <array>
#include <iterator>
#include <string_view>

using namespace std::literals;

// Just an illustration of where the semaphore/notification should be used
static bool notification = false;

static Ccf<{
    .rxBufSize = 256,
    .txBufSize = 256,
    .maxPktSize = 255,
}> ccf;

static std::array<uint8_t, 30> scratchLogBuf;
static std::span<uint8_t> scratchLogSpan{scratchLogBuf};

Rpc rpc
{
    Call{"add", "return x+y", {"x", "y"}, +[](int x, int y) { return x + y; }},
    Call{"hello", "greet the world", {}, +[](){ return "Hello, world!"sv; }},
    Call{"log", "test log", {},
    +[]()
    {
        ccf.logToBuffer(scratchLogSpan, LogLevel::Info, 0, "Test %d %f %s", 1, 2.0, "3");
    }},
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
        // Comms-CCF RPC "thread"
        if (notification)
        {
            notification = false;
            ccf.poll(rpc);
        }
        // Sending delayed logs in a different "thread", exclusive with
        // the comms-CCF RPC "thread" above as we don't do preemption
        // (not even for interrupts).
        if (scratchLogSpan.data() != scratchLogBuf.data())
        {
            const auto begin = &*scratchLogBuf.begin();
            const auto spanBegin = &*scratchLogSpan.begin();
            const auto spanEnd = &*scratchLogSpan.end();
            const auto end = &*scratchLogBuf.end();
            if (begin < spanBegin && spanEnd <= end)
            {
                std::span<uint8_t> logData{
                    begin,
                    static_cast<size_t>(std::distance(begin, spanBegin))
                };
                if (ccf.send(Channels::Log, logData))
                {
                    scratchLogSpan = scratchLogBuf;
                }
            }
            else
            {
                scratchLogSpan = scratchLogBuf;
            }
        }
        // Specific to this implementation: TX before getc to print
        // before blocking
        txIsr();
    }
}
