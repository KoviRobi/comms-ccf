#!/usr/bin/env python3

import sys
import time
from argparse import ArgumentParser
from asyncio import create_subprocess_exec, run
from asyncio.subprocess import PIPE
from pathlib import Path
from shlex import quote
from shutil import which

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

    proc = await create_subprocess_exec(
        executable, *args.arguments, stdin=PIPE, stdout=PIPE
    )
    assert proc.stdin is not None
    assert proc.stdout is not None
    transport = StreamTransport(
        proc.stdout, proc.stdin, sys.stderr if args.verbose else None
    )
    rpc = Rpc(transport)

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
    run(amain())
