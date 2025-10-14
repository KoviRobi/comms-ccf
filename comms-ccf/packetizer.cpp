#include "packetizer.hpp"

#include "circular_buffer.hpp"
#include "cobs.hpp"
#include "hexdump.hpp"

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <termios.h>

class UnbufferedStdio
{
public:
    UnbufferedStdio()
    {
        setvbuf(stdin, NULL, _IONBF, 0);
        setvbuf(stdout, NULL, _IONBF, 0);

        if (tcgetattr(0, &term_orig) == 0) {
            struct termios term = term_orig;
            term.c_lflag &= ~(ICANON | ECHO);
            tcsetattr(0, TCSANOW, &term);
        }
    }
    ~UnbufferedStdio()
    {
        tcsetattr(0, TCSANOW, &term_orig);
    }
private:
    struct termios term_orig;
};

constexpr size_t MAX_PKT_SIZE = 255;
static CircularBuffer<uint8_t, 256, MAX_PKT_SIZE> buf;
static Cobs::Decoder decoder{};
static bool notified = false;

void isr(uint8_t byte)
{
    // Do get before feed, and always do feed to e.g. reset on \0
    const uint8_t value = decoder.get(byte);
    const bool do_output = decoder.feed(byte);
    if (byte == 0)
    {
        buf.notify();
        notified = true;
    }
    else if (do_output)
    {
        buf.push_back(value);
    }
}

int main(int argc, char * argv[])
{
    auto _ = UnbufferedStdio();
    int i;
    while ( (i = fgetc(stdin)) != EOF)
    {
        uint8_t byte = static_cast<char>(i);
        isr(byte);
        // Main thread
        std::optional<decltype(buf)::Notification> notification;
        while (buf.get_notification(notification))
        {
            notified = false;
            uint8_t pkt[MAX_PKT_SIZE];
            size_t len = 0;
            for (auto c : *notification)
            {
                pkt[len++] = c;
            }
            printf("RX'd %zu\n", len);
            hexdump(std::span{pkt, len});
        }
    }
}
