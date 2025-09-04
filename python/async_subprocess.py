#!/usr/bin/env python3

import sys
import typing as t
from argparse import ArgumentParser
from asyncio import create_subprocess_exec, run
from asyncio.subprocess import PIPE
from pathlib import Path
from shlex import quote

from cbor import dumps
from transport import StreamTransport, Transport


async def call(transport: Transport, channel, index: int, args: t.Any) -> t.Any:
    await transport.send(channel, index.to_bytes() + dumps(args))
    response_channel, response = await transport.recv()
    assert channel == response_channel
    return response


async def amain():
    parser = ArgumentParser()
    parser.add_argument("executable", type=Path, help="Executable to wrap")
    parser.add_argument("--verbose", "-v", action="store_true", help="Dump packets")
    args = parser.parse_args()

    executable: Path = args.executable
    assert executable.exists(), f"File {quote(str(executable))} doesn't exist"

    proc = await create_subprocess_exec(executable, stdin=PIPE, stdout=PIPE)
    assert proc.stdin is not None
    assert proc.stdout is not None
    transport = StreamTransport(
        proc.stdout, proc.stdin, sys.stderr if args.verbose else None
    )

    print("schema ~>", await call(transport, 0, 0, []))
    print("add(2, 3) ~>", await call(transport, 0, 1, [2, 3]))

    proc.terminate()


if __name__ == "__main__":
    run(amain())
