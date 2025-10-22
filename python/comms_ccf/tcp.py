"""
Connects to a TCP Comms-CCF socket (plaintext)
"""

import asyncio
from argparse import ArgumentParser
from contextlib import asynccontextmanager


def command_parser(parser_or_subparser: ArgumentParser) -> ArgumentParser:
    parser_or_subparser.add_argument("--host", default="localhost", help="Host to connect to")
    parser_or_subparser.add_argument("--port", default=4321, type=int, help="Port to connect to")
    return parser_or_subparser

@asynccontextmanager
async def command(args):
    rx, tx = await asyncio.open_connection(args.host, args.port)
    yield rx, tx
    rx.feed_eof()
    tx.close()
