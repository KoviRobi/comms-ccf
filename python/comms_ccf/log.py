"""
Simple background logging task
"""

import io
from enum import Enum, auto

import cbor2
from cobs.cobs import DecodeError

from comms_ccf.channel import Channel, Channels


class LogLevel(Enum):
    Debug = 0
    Info = auto()
    Warn = auto()
    Error = auto()


async def print_logs(channels: Channels):
    channels.open_channel(Channel.Log)
    while True:
        try:
            data = await channels.recv(Channel.Log)
            if len(data) < 2:
                continue
            level = data[0] >> 5
            module = data[0] & 0x1F
            msgend = min(2 + data[1], len(data))
            msg = data[2:msgend]
            args = []
            argsIo = io.BytesIO(data[msgend:])
            try:
                # If there are any more arguments, decode them for formatting
                while True:
                    args.append(cbor2.load(argsIo))
            except cbor2.CBORDecodeEOF:
                pass
            try:
                formatted = msg.decode() % (*args,)
            except Exception:
                formatted = f"Error formatting log: {repr(msg)}"
            print(level, module, formatted)
        except (TimeoutError, DecodeError):
            pass
