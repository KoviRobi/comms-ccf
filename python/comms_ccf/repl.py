"""
Simple REPL with history and tab completion (if readline is present)
"""

import asyncio
import traceback
from pathlib import Path
from typing import AsyncGenerator

try:
    import readline
    import rlcompleter
except ModuleNotFoundError:
    readline = None
    rlcompleter = None


history_file = Path.home() / ".cache" / "comms-ccf_history"


class Stdio:
    async def input(self, *args, **kwargs):
        return await asyncio.to_thread(lambda: input(*args, **kwargs))

    async def print(self, *args, **kwargs):
        return await asyncio.to_thread(lambda: print(*args, **kwargs))


async def repl(io, locals):
    if readline and rlcompleter:
        completer = rlcompleter.Completer(namespace=locals)
        readline.set_completer(completer.complete)
        if readline.backend == "readline":
            readline.parse_and_bind("tab: complete")
        else:
            readline.parse_and_bind("python:bind ^I rl_complete")

    if readline:
        if not history_file.exists():
            history_file.touch()
        readline.read_history_file(str(history_file))

    while True:
        try:
            line = await io.input("in>  ")
            line = line.strip()
            if readline:
                readline.append_history_file(1000, str(history_file))
        except (KeyboardInterrupt, EOFError):
            break
        if not line:  # Empty line
            continue
        try:
            # Patch line to be able to use await inside byn making it an
            # (async) generator
            line = f"((\n{line}\n) for _ in '_')"
            expr = eval(line, locals, locals)
            if isinstance(expr, AsyncGenerator):
                expr = await expr.__anext__()
            else:
                expr = next(expr)
            if asyncio.iscoroutine(expr):
                expr = await expr
            if expr is not None:
                await io.print("out>", expr)
        except Exception:
            await io.print(traceback.format_exc())
