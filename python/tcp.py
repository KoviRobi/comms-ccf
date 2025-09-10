#!/usr/bin/env python3

import asyncio
import signal
import sys
import time
from argparse import ArgumentParser

from cobs.cobs import DecodeError

from channel import Channels
from repl import repl
from rpc import Rpc
from transport import StreamTransport

background_tasks = set()

class Stdio:
    input = input
    print = print

async def amain():
    parser = ArgumentParser()
    parser.add_argument("--host", default="localhost", help="Host to connect to")
    parser.add_argument("--port", default=4321, type=int, help="Port to connect to")
    parser.add_argument("--verbose", "-v", action="store_true", help="Dump packets")
    parser.add_argument("--no-repl", "-n", action="store_true", help="Only try example")
    args = parser.parse_args()

    global rx, tx, transport, rpc
    rx, tx = await asyncio.open_connection(args.host, args.port)
    transport = StreamTransport(rx, tx, log_fp=sys.stderr if args.verbose else None)
    loop = asyncio.get_event_loop()
    channels = Channels(transport, loop)
    channels.open_channel(0)
    channel_task = loop.create_task(channels.loop())
    background_tasks.add(channel_task)
    channel_task.add_done_callback(background_tasks.discard)
    rpc = Rpc(channels)

    async def log():
        channels.open_channel(1)
        while True:
            try:
                msg = await channels.recv(1)
                print("Log: ", msg.decode("ascii", "ignore"))
            except (TimeoutError, DecodeError):
                pass

    log_task = loop.create_task(log())
    background_tasks.add(log_task)
    log_task.add_done_callback(background_tasks.remove)

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

    await repl(Stdio(), locals)


if __name__ == "__main__":
    # Make keyboard interrupt quit AsyncIO
    signal.signal(signal.SIGINT, (lambda *_: exit(0)))
    asyncio.run(amain())
