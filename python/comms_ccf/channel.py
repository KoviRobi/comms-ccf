"""
# Channelizer

This code splits up the incoming data into several channels, so that
code on top of this can easily ignore messages not intended for it.

The basic idea is to use a dictionary of futures to wait for, which are
set when a value arrives on that channel.

The channels only remember the latest message. An alternative could be
using a queue instead of futures.
"""

import pdb
from asyncio import Future, IncompleteReadError, get_event_loop, shield, wait_for
from enum import IntEnum

from comms_ccf.transport import DEFAULT_TIMEOUT, Transport


class Channel(IntEnum):
    RPC = 0
    Log = 1


class Channels:
    def __init__(self, transport: Transport, loop=get_event_loop()) -> None:
        self._transport = transport
        self._channels: dict[int, Future[bytes] | None] = {}
        self._loop = loop

    def open_channel(self, channel: int):
        if channel not in self._channels:
            self._channels[channel] = self._loop.create_future()

    async def loop(self, debug=False):
        while True:
            try:
                chan, data = await self._transport.recv()
                if chan in self._channels:
                    fut = self._channels[chan]
                    if fut is None:
                        continue
                    if fut.done():
                        fut = self._loop.create_future()
                        fut.set_result(data)
                        self._channels[chan] = fut
                    else:
                        fut.set_result(data)
                else:
                    name = chan
                    if chan in Channel:
                        name = Channel(chan).name
                    print("Dropping channel", name, "data", data)
                    print("future messages ignored on channel", name)
                    self._channels[chan] = None
            except TimeoutError:
                pass
            except IncompleteReadError:
                break
            except Exception as e:
                print("Exception in channel", str(e) or repr(e))
                if debug:
                    pdb.post_mortem(e.__traceback__)

    async def send(
        self, channel: int, data: bytes, *, timeout: float = DEFAULT_TIMEOUT
    ):
        await self._transport.send(channel, data, timeout=timeout)

    async def recv(self, channel: int, *, timeout: float = DEFAULT_TIMEOUT) -> bytes:
        fut = self._channels.get(channel)
        if fut is None:
            fut = self._loop.create_future()
            self._channels[channel] = fut
        result = await wait_for(shield(fut), timeout=timeout)
        fut = self._loop.create_future()
        self._channels[channel] = fut
        return result
