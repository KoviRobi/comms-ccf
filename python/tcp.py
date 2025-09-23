#!/usr/bin/env python3

"""
Connects to a TCP Comms-CCF socket

TODO: De-duplicate along with wrap_subprocess.py
"""

import asyncio
import signal
import sys
import time
from argparse import ArgumentParser

from background import BackgroundTasks
from channel import Channels
from log import print_logs
from repl import repl
from rpc import Rpc
from transport import StreamTransport


class Stdio:
    input = input
    print = print


async def amain():
    parser = ArgumentParser()
    parser.add_argument("--host", default="localhost", help="Host to connect to")
    parser.add_argument("--port", default=4321, type=int, help="Port to connect to")
    parser.add_argument("--verbose", "-v", action="store_true", help="Dump packets")
    parser.add_argument(
        "--no-repl", "-n", dest="repl", action="store_false", help="Only try example"
    )
    parser.add_argument(
        "--no-log", "-N", dest="log", action="store_false", help="Don't output logs"
    )
    args = parser.parse_args()

    global rx, tx, transport, rpc
    rx, tx = await asyncio.open_connection(args.host, args.port)
    transport = StreamTransport(rx, tx, log_fp=sys.stderr if args.verbose else None)
    loop = asyncio.get_event_loop()
    background_tasks = BackgroundTasks(loop)
    channels = Channels(transport, loop)
    channels.open_channel(0)
    background_tasks.add(channels.loop)
    rpc = Rpc(channels)

    if args.log:
        background_tasks.add(print_logs, channels)

    while True:
        try:
            await rpc.discover()
            break
        except Exception as e:
            print(e)
            time.sleep(0.2)

    locals = {k: v for k, v in rpc.methods().items()}
    locals["help"] = rpc.help
    locals["dir"] = dir

    print("Use help(name=None) for discovered methods")
    print("(optionally name to document just that method)")
    try:
        print("E.g. add(2,3) ~>", await rpc.add(2, 3))
    except Exception as e:
        print(e)

    if args.repl:
        await repl(Stdio(), locals)


def quit(*args, **kwargs):
    sys.stdin.close()
    print("Press Ctrl-D (or any other key) to exit")
    exit()


if __name__ == "__main__":
    # Make keyboard interrupt quit AsyncIO
    signal.signal(signal.SIGINT, quit)
    asyncio.run(amain())
