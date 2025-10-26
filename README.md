[![Tests](https://github.com/KoviRobi/comms-ccf/actions/workflows/test.yaml/badge.svg)
<br/><sup>(click for more details)</sup>
](https://kovirobi.github.io/comms-ccf/test.html)

# Comms-CCF

This is a simple communication layer based on COBS, CBOR and FNV-1A

Can be used to implement other channels on top, e.g. RPC, logging,
tracing etc.

None of the ideas are new here, but it's still useful to have them
together like this. And it's more focused on embedded than some other
libraries.  (At the same time, the code size could perhaps be smaller,
I used C++ templates and they can expand to a fair amount of
code. Especially when  using no optimisation [`-O0`] instead of debug
optimisation [`-Og`].)

## Status: Proof of concept

Currently there is a working demo in [FreeRTOS-Demo/](/FreeRTOS-Demo/)
which should be possible to port to other platforms. There is some Python
support in [python/comms_ccf/](python/comms_ccf/) which can connect to
the demo and issue RPC commands.

[![asciicast](https://asciinema.org/a/740327.svg)](https://asciinema.org/a/740327)

This is proof-of-concept because not all of the layers below are filled
in properly:

1. [ ] Python layer could use IPython for a more friendly REPL.
2. [ ] RPC
    1. [X] The Python type hints only do simple types, doing nested
    types such as tuples is not yet done.
    2. [X] Allow functions returning void values
    3. [ ] More complicated RPC is not done (e.g. asynchronous functions,
    functions returning data over several packets).
    4. [ ] Storing values (e.g. objects) across function calls is not
    done -- probably not necessary though.
    5. [ ] Proper error values for issues with the RPC.
3. [ ] Other protocols such as ~logging~ or streaming sensor results
not yet demonstrated.
   1. [X] Basic logging done.
   2. [X] Deferred format logging to avoid pulling in printf (and compare
   space difference).
   3. [ ] Only sending pointers to rodata string constants.
   4. [ ] Streaming events to plot, e.g. ADC readings (though maybe random
   data for qemu).
   5. [ ] Streaming trace events.
4. [ ] CBOR
   1. [ ] The API needs improving to be able to avoid doing all the
   decoding on the stack as a return value. For example, read/write
   memory doesn't need to materialise all of the bytes on the stack,
   it could use an iterator for decode, and a span for encode.
   2. [ ] Supporting structs is not currently done. The constructor
   could probably be used as an RPC function, so constructing could be
   done, but destructing is more tricky. One solution for POD
   structs is to infer fields from default constructor, see
   <https://github.com/Mizuchi/ForeachMember>. Or have a way for users
   to define serialization of their struct.
5. [ ] The circular buffer has a few TODOs to improve it.
6. [ ] The COBS layer could have input & output iterators, which could
then be used to have views-like interfaces and make defining the CCF
layer neater.


## Demo & Resource use

There is a demo in the [FreeRTOS-Demo/](/FreeRTOS-Demo/) folder,
with detailed information about where the flash/memory use comes from.

A summary of the resurce use is copied below from the
[FreeRTOS-Demo/README.md](/FreeRTOS-Demo/README.md) file.

<!-- Table starts here -->

| Summary                                | .text                         | .bss                        | .data             |
|----------------------------------------|-------------------------------|-----------------------------|-------------------|
| No Comms-CCF                           | 11.15KiB                      | 3.53KiB                     | 356B              |
| Add Comms-CCF, no RPC handlers         | [13.21KiB (+2.06Ki)][.text1]  | [5.62KiB (+2.09Ki)][.bss1]  | 356B (no change)  |
| Add version RPC call                   | [14.09KiB (+900)][.text2]     | [5.64KiB (+24)][.bss2]      | 356B (no change)  |
| Add `add` call                         | [14.80KiB (+728)][.text3]     | [5.67KiB (+28)][.bss3]      | 356B (no change)  |
| Add `sub` call, similar to `add`       | [14.86KiB (+60)][.text4]      | [5.70KiB (+28)][.bss4]      | 356B (no change)  |
| Add `greet` call                       | [15.16KiB (+308)][.text5]     | [5.72KiB (+24)][.bss5]      | 356B (no change)  |
| Add `readm_mem` and `write_mem` calls  | [16.55KiB (+1.39Ki)][.text6]  | [5.78KiB (+56)][.bss6]      | 356B (no change)  |
| Add test log task                      | [20.66KiB (+4.11Ki)][.text7]  | [6.86KiB (+1.09Ki)][.bss7]  | 356B (no change)  |
| Inline vtable                          | [20.62KiB (-32)][.text8]      | [6.89KiB (+24)][.bss8]      | 356B (no change)  |
| Deferred formatting                    | [19.71KiB (-932)][.text9]     | [6.86KiB (-24)][.bss9]      | 356B (no change)  |

[.text1]: https://kovirobi.github.io/comms-ccf/compare.0-1.svg#.text
[.text2]: https://kovirobi.github.io/comms-ccf/compare.1-2.svg#.text
[.text3]: https://kovirobi.github.io/comms-ccf/compare.2-3.svg#.text
[.text4]: https://kovirobi.github.io/comms-ccf/compare.3-4.svg#.text
[.text5]: https://kovirobi.github.io/comms-ccf/compare.4-5.svg#.text
[.text6]: https://kovirobi.github.io/comms-ccf/compare.5-6.svg#.text
[.text7]: https://kovirobi.github.io/comms-ccf/compare.6-7.svg#.text
[.text8]: https://kovirobi.github.io/comms-ccf/compare.7-8.svg#.text
[.text9]: https://kovirobi.github.io/comms-ccf/compare.8-9.svg#.text
[.bss1]: https://kovirobi.github.io/comms-ccf/compare.0-1.svg#.bss
[.bss2]: https://kovirobi.github.io/comms-ccf/compare.1-2.svg#.bss
[.bss3]: https://kovirobi.github.io/comms-ccf/compare.2-3.svg#.bss
[.bss4]: https://kovirobi.github.io/comms-ccf/compare.3-4.svg#.bss
[.bss5]: https://kovirobi.github.io/comms-ccf/compare.4-5.svg#.bss
[.bss6]: https://kovirobi.github.io/comms-ccf/compare.5-6.svg#.bss
[.bss7]: https://kovirobi.github.io/comms-ccf/compare.6-7.svg#.bss
[.bss8]: https://kovirobi.github.io/comms-ccf/compare.7-8.svg#.bss
[.bss9]: https://kovirobi.github.io/comms-ccf/compare.8-9.svg#.bss

<!-- Table ends here -->

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

The format on the wire is the following, little endian network byte
order for the checksum because that is what I'm used to writing.

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

## Tweaks/defines
- `DEFERRED_FORMATTING` (or host-side formatting) means you can avoid
doing `printf` on the micro, which might save some code space.
