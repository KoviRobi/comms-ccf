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
from asyncio import (
    IncompleteReadError,
    Queue,
    QueueShutDown,
    get_event_loop,
)
from enum import IntEnum

from comms_ccf.transport import DEFAULT_TIMEOUT, Transport


class Channel(IntEnum):
    RPC = 0
    Log = 1


class Channels:
    def __init__(self, transport: Transport, loop=get_event_loop()) -> None:
        self._transport = transport
        self._channels: dict[int, Queue[bytes] | None] = {}
        self._loop = loop
        self._exc: None | BaseException = None

    def open_channel(self, channel: int, maxsize=0):
        queue = self._channels.get(channel)
        if queue is None:
            self._channels[channel] = Queue(maxsize=maxsize)

    async def loop(self, debug=False):
        while True:
            try:
                chan, data = await self._transport.recv()
                queue = self._channels.get(chan)
                if queue is not None:
                    await queue.put(data)
                else:
                    name = chan
                    if chan in Channel:
                        name = Channel(chan).name
                    print("Dropping channel", name, "data", data)
                    print("future messages ignored on channel", name)
                    self._channels[chan] = None
            except TimeoutError:
                pass
            except (IncompleteReadError, EOFError) as e:
                self._exc = e
                for queue in self._channels.values():
                    if queue is not None:
                        queue.shutdown(immediate=False)
                break
            except BaseException as e:
                print("Exception in channel", str(e) or repr(e))
                if debug:
                    pdb.post_mortem(e.__traceback__)

    async def send(
        self, channel: int, data: bytes, *, timeout: float = DEFAULT_TIMEOUT
    ):
        await self._transport.send(channel, data, timeout=timeout)

    async def recv(self, channel: int, *, timeout: float = DEFAULT_TIMEOUT) -> bytes:
        queue = self._channels.get(channel)
        assert queue is not None, f"Must call open first on channel: {channel}"
        try:
            data = await queue.get()
            return data
        except QueueShutDown:
            pass  # Don't nest the exception
        if self._exc is not None:
            raise self._exc
        assert False, "Logic error: _exc and queue.shutdown not linked"
