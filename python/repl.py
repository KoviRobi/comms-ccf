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


async def repl(globals, locals):
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
            line = input("in>  ").strip()
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
            print("out> ", end="")
            expr = eval(line, locals, locals)
            if isinstance(expr, AsyncGenerator):
                expr = await expr.__anext__()
            else:
                expr = next(expr)
            if asyncio.iscoroutine(expr):
                print(await expr)
            else:
                print(expr)
        except Exception:
            traceback.print_exc()
