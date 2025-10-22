"""
An alternative to the TCP transport, it wraps the STDIO of a subprocess.
"""

import asyncio
from contextlib import asynccontextmanager
from argparse import ArgumentParser
from asyncio.subprocess import PIPE
from pathlib import Path
from shlex import quote
from shutil import which


def command_parser(parser_or_subparser: ArgumentParser) ->ArgumentParser:
    parser_or_subparser.add_argument("executable", type=Path, help="Executable to wrap")
    parser_or_subparser.add_argument(
        "arguments", type=str, nargs="*", help="Arguments to the executable"
    )
    return parser_or_subparser

@asynccontextmanager
async def command(args):
    executable: Path = Path(which(args.executable))
    assert executable.exists(), f"File {quote(str(executable))} doesn't exist"

    proc = await asyncio.create_subprocess_exec(
        executable, *args.arguments, stdin=PIPE, stdout=PIPE
    )
    assert proc.stdin is not None
    assert proc.stdout is not None
    yield (proc.stdout, proc.stdin)
    proc.terminate()
