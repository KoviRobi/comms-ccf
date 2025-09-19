from asyncio import Future, IncompleteReadError, get_event_loop, shield, wait_for
from enum import IntEnum

from transport import DEFAULT_TIMEOUT, Transport


class Channel(IntEnum):
    RPC = 0
    Log = 1


class Channels:
    def __init__(self, transport: Transport, loop=get_event_loop()) -> None:
        self._transport = transport
        self._channels: dict[int, Future[bytes]] = {}
        self._loop = loop

    def open_channel(self, channel: int):
        if channel not in self._channels:
            self._channels[channel] = self._loop.create_future()

    async def loop(self):
        while True:
            try:
                chan, data = await self._transport.recv()
                if chan in self._channels:
                    fut = self._channels[chan]
                    if fut.done():
                        fut = self._loop.create_future()
                        fut.set_result(data)
                        self._channels[chan] = fut
                    else:
                        self._channels[chan].set_result(data)
                else:
                    print("Dropping data", data)
            except IncompleteReadError:
                break
            except Exception as e:
                print("Exception", e)

    async def send(
        self, channel: int, data: bytes, *, timeout: float = DEFAULT_TIMEOUT
    ):
        await self._transport.send(channel, data, timeout=timeout)

    async def recv(self, channel: int, *, timeout: float = DEFAULT_TIMEOUT) -> bytes:
        result = await wait_for(shield(self._channels[channel]), timeout=timeout)
        self._channels[channel] = self._loop.create_future()
        return result
