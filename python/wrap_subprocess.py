#!/usr/bin/env python3

"""
An alternative to the TCP transport, it wraps the STDIO of a subprocess.

TODO: De-duplicate along with tcp.py
"""

import asyncio
import sys
import time
from argparse import ArgumentParser
from asyncio.subprocess import PIPE
from pathlib import Path
from shlex import quote
from shutil import which

from background import BackgroundTasks
from channel import Channels
from log import print_logs
from repl import Stdio, repl
from rpc import Rpc
from transport import StreamTransport


async def amain():
    parser = ArgumentParser()
    parser.add_argument("executable", type=Path, help="Executable to wrap")
    parser.add_argument(
        "arguments", type=str, nargs="*", help="Arguments to the executable"
    )
    parser.add_argument("--verbose", "-v", action="store_true", help="Dump packets")
    parser.add_argument(
        "--no-repl", "-n", dest="repl", action="store_false", help="Only try example"
    )
    parser.add_argument(
        "--no-log", "-N", dest="log", action="store_false", help="Don't output logs"
    )
    args = parser.parse_args()

    executable: Path = Path(which(args.executable))
    assert executable.exists(), f"File {quote(str(executable))} doesn't exist"

    proc = await asyncio.create_subprocess_exec(
        executable, *args.arguments, stdin=PIPE, stdout=PIPE
    )
    assert proc.stdin is not None
    assert proc.stdout is not None
    transport = StreamTransport(
        proc.stdout, proc.stdin, sys.stderr if args.verbose else None
    )
    loop = asyncio.get_event_loop()
    background_tasks = BackgroundTasks(loop)
    channels = Channels(transport, loop)
    background_tasks.add(channels.loop)
    rpc = Rpc(channels)

    if args.log:
        background_tasks.add(print_logs, channels)

    while True:
        try:
            await rpc.discover()
            break
        except Exception as e:
            print("Failed to discover RPC:", str(e) or repr(e))
            time.sleep(0.2)
    locals = {k: v for k, v in rpc.methods().items()}
    locals["help"] = rpc.help
    locals["dir"] = dir

    print("Use help(name=None) for discovered methods")
    print("(optionally name to document just that method)")
    try:
        print("E.g. add(2,3) ~>", await rpc.add(2, 3))
    except Exception as e:
        print("Exception in demo:", str(e) or repr(e))

    if args.repl:
        await repl(Stdio(), locals)

    proc.terminate()


if __name__ == "__main__":
    asyncio.run(amain())
