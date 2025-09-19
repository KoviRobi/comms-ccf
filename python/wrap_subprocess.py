#!/usr/bin/env python3

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
from repl import repl
from rpc import Rpc
from transport import StreamTransport


async def amain():
    parser = ArgumentParser()
    parser.add_argument("executable", type=Path, help="Executable to wrap")
    parser.add_argument(
        "arguments", type=str, nargs="*", help="Arguments to the executable"
    )
    parser.add_argument("--verbose", "-v", action="store_true", help="Dump packets")
    parser.add_argument("--no-repl", "-n", action="store_true", help="Only try example")
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
    channels.open_channel(0)
    background_tasks.add(channels.loop)
    rpc = Rpc(channels)

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

    if not args.no_repl:
        await repl(locals, locals)

    proc.terminate()


if __name__ == "__main__":
    asyncio.run(amain())
