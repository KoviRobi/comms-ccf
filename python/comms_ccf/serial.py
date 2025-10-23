"""
Connects to a serial port
"""

import serial_asyncio
from argparse import ArgumentParser
from contextlib import asynccontextmanager
from pathlib import Path


def command_parser(parser_or_subparser: ArgumentParser) -> ArgumentParser:
    parser_or_subparser.add_argument("port", type=Path, help="Port to connect to")
    parser_or_subparser.add_argument("--baud", default=115200, help="Baud rate, defaults to 115200")
    return parser_or_subparser

@asynccontextmanager
async def command(args):
    rx, tx = await serial_asyncio.open_serial_connection(url=str(args.port), baudrate=args.baud)
    yield rx, tx
    rx.feed_eof()
    tx.close()

