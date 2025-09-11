import typing as t
from asyncio import StreamReader, StreamWriter

from cobs.cobs import DecodeError, decode, encode
from fnv_hash_fast import fnv1a_32
from hexdump import hexdump

# Float (seconds)
DEFAULT_TIMEOUT = 0.2

# Channel index (1) + Checksum (4) + b"\0"
MIN_PKT_SIZE = 6


class Transport(t.Protocol):
    async def send(
        self, channel: int, data: bytes, *, timeout: float = DEFAULT_TIMEOUT
    ):
        "Sends bytes over the given channel as a frame."
        ...

    async def recv(self, *, timeout: float = DEFAULT_TIMEOUT) -> tuple[int, bytes]:
        "Returns the data from the received frame, minus channel & checksum."
        ...


class StreamTransport:
    def __init__(
        self, rx: StreamReader, tx: StreamWriter, log_fp: t.Optional[t.TextIO] = None
    ) -> None:
        self._tx = tx
        self._rx = rx
        self._log = log_fp

    async def send(
        self, channel: int, data: bytes, *, timeout: float = DEFAULT_TIMEOUT
    ):
        data = int.to_bytes(channel) + data
        data = data + fnv1a_32(data).to_bytes(length=4, byteorder="little")
        data = encode(data) + b"\0"
        if self._log:
            print(hexdump(data, "TX: "), file=self._log)
        self._tx.write(data)
        await self._tx.drain()

    async def recv(self, *, timeout: float = DEFAULT_TIMEOUT) -> tuple[int, bytes]:
        # TODO: timeouts, MAX_PKT_SIZE
        data = await self._rx.readuntil(b"\0")
        if self._log:
            print(hexdump(data, "RX: "), file=self._log)
        assert len(data) >= MIN_PKT_SIZE, "Packet too small\n" + hexdump(data, "pkt> ")
        assert data[-1] == 0, "Missing terminator\n" + hexdump(data, "pkt> ")
        try:
            decoded = decode(data[:-1])
        except DecodeError as e:
            print(str(e) + "\n" + hexdump(data, "pkt> "))
            raise
        checksum = int.from_bytes(decoded[-4:], byteorder="little")
        expected = fnv1a_32(decoded[:-4])
        assert (
            expected == checksum
        ), f"Bad checksum {checksum:08X} expected {expected:08X}\n" + hexdump(
            decoded, "pkt> "
        )
        channel = decoded[0]
        return (channel, decoded[1:-4])
