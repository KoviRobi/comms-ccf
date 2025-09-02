# Comms-CCF

This is a simple communication layer based on COBS, CBOR and FNV-1A

Can be used to implement other channels on top, e.g. RPC, logging,
tracing etc.

None of the ideas are new here, but it's still useful to have them
together like this. And it's perhaps more focused on embedded than
some other libraries.  (At the same time, the code size could perhaps
be smaller, I used C++ templates and they can expand to a fair amount
of code.)

## Acknowledgements

With much gratitude to Robert McGregor and Jamie Wood, who have shown
me that writing something like this can be easy and fun. And showing
the utility in writing something like this compared to continuing on
with the standard "write a CLI over UART" method.

Their method was different, using a Python program to parse and compile a
DSL for describing the RPC calls. This allowed that to be used easily with
C, and also support more features and fewer workarounds to the template
limitations. I used C++ templates to continue my learning of them.

## Expected use

The expected use of this is between a microcontroller and a host-machine,
communicating over e.g. UART or similar (byte) serial channel. The aim
is to make a few things easy:
- Controlling the embedded device (alternative to serial command-line,
  with support for history, tab-completion, all on the host side);
- In the same vein, automating that control for testing, without having to
  write command-line parsers (which potentially have to deal with the
  lack of flow-control [e.g. ready-to-send/clear-to-send] signals);
- Multiplexing the channel for different streams, e.g. different logging
  components.

This impacts design in a couple of ways:

- CPU is much faster than the transport, so we can do checksumming as
  we copy the received byte

- Channel can be connected/disconnected, so need a framing;

- Channel isn't 100% error free, due to glitches during
  connect/disconnect; but is considered reliable otherwise;

- Channel delivers packets in order, and has low latency (rather, the
  product of latency and speed is small, so negligible amount of data
  sent but not yet received) so no need to have multiple packets in
  flight at once;

- These two mean we can assume a FIFO model, with maybe the level above
  implementing ack/retry, but a simple check is fine, no need for
  error-correcting codes;

- Packets are received from an interrupt handler, that needs to queue
  them for deferred processing.

  Specifically it might need to enqueue a packet while some packets
  are being processed. Not seeing the newly added packet is fine (the
  interrupt will notify it to try again), but it cannot use a mutex
  because that would leave the interrupt being blocked on the thread
  (probably a crash but at least a priority inversion);

- Packets are sent from multiple threads, but processed by a single
  transmit task which might be busy (UART isn't so fast after all)
  so need a transmit queue also

## Transport wire format

The format on the wire is the following, big endian network byte order for the
checksum because the CBOR is also big endian.

| `ID: u8` | `data: u8[]/CBOR` | `checksum: u32` |
|----------|-------------------|-----------------|

The ID is a channel ID/tag, the data is a variable length of bytes (the
length is given by the framing layer), and the checksum is the FNV-1A
checksum, over both the ID and data in that order.

This isn't the traditional CRC-32 (Ethernet variant) because FNV-1A uses
fewer bytes for the same throughput (no precomputed table), is very
easy to implement and is pretty good. There are better algorithms for
absolute throughput and randomness (say SipHash, Murmur2), but these have
usually been written for throughput on a desktop with vector extensions,
and their code does end up being a little bigger.

Given the use case here of communicating between two trusted parties, the
choice of FNV-1A seems good. It does operate on a byte byte basis, which does
slow it down but also means we don't impose length/padding requirements on the
messages.

Further, the data might be encoded as CBOR (depending on the ID). For example
if the ID is for a serial channel, unstructured bytes are fine. But for logs,
structured data can be useful (and not just for the metatdata). For RPC,
structured data is necessary.

I chose CBOR because it is easy to encode/decode into a compact format,
supports many types (more than I end up using).
