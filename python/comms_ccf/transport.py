"""
This receives packets from an async channel and decodes the COBS encoding,
checks the checksum.
"""

import asyncio
import time
import typing as t

from cobs.cobs import DecodeError, decode, encode
from fnv_hash_fast import fnv1a_32

from comms_ccf.hexdump import hexdump

# Float (seconds)
DEFAULT_TIMEOUT = 0.5

# Channel index (1) + Checksum (4) + b"\0"
MIN_PKT_SIZE = 6
MAX_PKT_SIZE = 256


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
        self,
        rx: asyncio.StreamReader,
        tx: asyncio.StreamWriter,
        log_fp: t.Optional[t.TextIO] = None,
    ) -> None:
        self._tx = tx
        self._rx = rx
        self._rxBuf = b""
        self._log = log_fp

    async def send(
        self, channel: int, data: bytes, *, timeout: float = DEFAULT_TIMEOUT
    ):
        data = int.to_bytes(channel) + data
        data = data + fnv1a_32(data).to_bytes(length=4, byteorder="little")
        data = encode(data) + b"\0"
        if self._log:
            print(hexdump(data, "TX: "), file=self._log)
        async with asyncio.timeout(timeout):
            self._tx.write(data)
            await self._tx.drain()

    async def recv(self, *, timeout: float = DEFAULT_TIMEOUT) -> tuple[int, bytes]:
        async with asyncio.timeout(timeout):
            while True:
                idx = self._rxBuf.find(0)
                if idx > 0:
                    data = self._rxBuf[: idx + 1]
                    self._rxBuf = self._rxBuf[idx + 1 :]
                    break
                else:
                    self._rxBuf += await self._rx.read(MAX_PKT_SIZE)
                    assert len(self._rxBuf) < MAX_PKT_SIZE, "Packet max size exceeded"
        if self._log:
            print(hexdump(data, "RX: "), file=self._log)
        assert len(data) >= MIN_PKT_SIZE, "Packet too small\n" + hexdump(data, "pkt> ")
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
