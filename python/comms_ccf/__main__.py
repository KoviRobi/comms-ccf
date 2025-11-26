#!/usr/bin/env python3

"""
Connects to a socket, process, or serial port.
"""

import asyncio
import signal
import sys
import time
import typing as t
from argparse import ArgumentParser
from pathlib import Path

from comms_ccf import rtt, serial, stdio, tcp
from comms_ccf.background import BackgroundTasks
from comms_ccf.channel import Channels
from comms_ccf.hexdump import hexdump
from comms_ccf.log import print_logs
from comms_ccf.repl import Stdio, repl, script
from comms_ccf.rpc import Rpc
from comms_ccf.transport import StreamTransport


async def demo_rpc(rpc: Rpc):
    if add := getattr(rpc, "add", None):
        try:
            print("E.g. add(2,3) ~>", await add(2, 3))
        except Exception as e:
            print("Exception in demo:", str(e) or repr(e))


async def init_locals(rpc: Rpc):
    while True:
        try:
            await rpc.discover()
            break
        except EOFError:
            raise  # No point in trying again
        except Exception as e:
            print("Failed to discover RPC:", str(e) or repr(e))
            time.sleep(0.2)

    locals = {k: v for k, v in rpc.methods().items()}
    locals["_call"] = lambda n, *args, **kwargs: rpc(n, args, **kwargs)
    locals["help"] = rpc.help
    locals["hexdump"] = hexdump
    locals["dir"] = dir

    print("Use help(name=None) for discovered methods")
    print("(optionally name to document just that method)")
    return locals


async def amain():
    parser = ArgumentParser()
    parser.add_argument("--verbose", "-v", action="store_true", help="Dump packets")
    parser.add_argument(
        "--no-repl", "-n", dest="repl", action="store_false", help="Only try example"
    )
    parser.add_argument(
        "--no-log", "-N", dest="log", action="store_false", help="Don't output logs"
    )
    parser.add_argument("--expect-logs", type=Path, help="Check logs against file")
    parser.add_argument(
        "--script-file", type=Path, help="Check `> command` gives `< value`"
    )
    parser.add_argument(
        "--debug", "-d", action="store_true", help="Open debugger on exceptions"
    )
    sp = parser.add_subparsers(
        description="Subcommands, see `%(prog)s <subcommand> --help`", required=True
    )

    tcp_parser = tcp.command_parser(sp.add_parser("tcp"))
    tcp_parser.set_defaults(func=tcp.command)

    stdio_parser = stdio.command_parser(sp.add_parser("stdio"))
    stdio_parser.set_defaults(func=stdio.command)

    serial_parser = serial.command_parser(sp.add_parser("serial"))
    serial_parser.set_defaults(func=serial.command)

    rtt_parser = rtt.command_parser(sp.add_parser("rtt"))
    rtt_parser.set_defaults(func=rtt.command)

    args = parser.parse_args()
    func: t.Callable[
        [t.Any],
        t.AsyncContextManager[tuple[asyncio.StreamReader, asyncio.StreamWriter], t.Any],
    ] = args.func
    async with func(args) as context:
        rx, tx = context

        transport = StreamTransport(rx, tx, log_fp=sys.stderr if args.verbose else None)
        loop = asyncio.get_event_loop()
        channels = Channels(transport, loop)
        rpc = Rpc(channels)

        background_tasks = BackgroundTasks(loop)
        try:
            # First open the log channel before starting the channels loop
            if args.log:
                background_tasks.add(print_logs, channels, args.expect_logs)

            background_tasks.add(channels.loop, args.debug)

            if args.repl or args.script_file:
                locals = await init_locals(rpc)
                await demo_rpc(rpc)

                if args.script_file:
                    errors = await script(args.script_file, locals)
                    if errors:
                        raise SystemExit(errors)

                if args.repl:
                    await repl(Stdio(), locals, args.debug)

        finally:
            background_tasks.suppress_exceptions.add(EOFError)
            background_tasks.suppress_exceptions.add(asyncio.CancelledError)
            tx.close()
            await background_tasks.wait(timeout=5)


def quit(*args, **kwargs):
    sys.stdin.close()
    print("Press Ctrl-D (or any other key) to exit")
    exit()


def main():
    # Make keyboard interrupt quit AsyncIO
    signal.signal(signal.SIGINT, quit)
    asyncio.run(amain())


if __name__ == "__main__":
    main()
