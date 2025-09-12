"""
Simple background logging task
"""

from enum import Enum, auto
import typing as t

from channel import Channels

from cobs.cobs import DecodeError

class LogLevel(Enum):
    Debug = 0
    Info = auto()
    Warn = auto()
    Error = auto()

async def print_logs(channels: Channels, output: t.Callable[[str, int, str], t.Awaitable]):
    channels.open_channel(1)
    while True:
        try:
            data = await channels.recv(1)
            if len(data) < 2:
                continue
            level = data[0] >> 5
            module = data[0] & 0x1F
            msg = data[1:]
            await output(LogLevel(level).name, module, msg.decode())
        except (TimeoutError, DecodeError):
            pass
