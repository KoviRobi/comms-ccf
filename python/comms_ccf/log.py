"""
Simple background logging task
"""

import difflib
import io
from enum import Enum, auto
from pathlib import Path

import cbor2
from cobs.cobs import DecodeError

from comms_ccf.channel import Channel, Channels


class LogLevel(Enum):
    Debug = 0
    Info = auto()
    Warn = auto()
    Error = auto()


async def print_logs(channels: Channels, expect_logs: Path | None = None):
    channels.open_channel(Channel.Log)

    expected_logs = None
    got_logs = []
    if expect_logs is not None:
        expected_logs = [
            line
            for line in expect_logs.open("rt")
            if line != "\n" and not line.startswith("#")
        ]

    while True:
        try:
            data = await channels.recv(Channel.Log)
            if len(data) < 2:
                continue
            level = LogLevel(data[0] >> 5).name
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
            if expected_logs is not None:
                got_logs.append(f"{level} {module} {formatted}\n")
        except (TimeoutError, DecodeError):
            pass
        except EOFError:
            if expected_logs is not None:
                assert expected_logs == got_logs, "Logs not as expected:\n" + "".join(
                    difflib.unified_diff(
                        a=expected_logs,
                        b=got_logs,
                        fromfile="Expected",
                        tofile="Got",
                    )
                )
            raise
