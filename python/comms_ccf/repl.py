"""
Simple REPL with history and tab completion (if readline is present)
"""

import asyncio
import io
import pdb
import re
import sys
import traceback
from dataclasses import dataclass
from pathlib import Path
from typing import AsyncGenerator, Protocol

try:
    import readline
    import rlcompleter
except ModuleNotFoundError:
    readline = None
    rlcompleter = None


history_file = Path.home() / ".cache" / "comms-ccf_history"


class Console(Protocol):
    def close(self, reason: str) -> None: ...
    async def input(self, prompt: str = "") -> str: ...
    async def print(self, *strs: str, sep: str = " ", end: str = "\n") -> None: ...


class Stdio:
    def __init__(self, loop: asyncio.AbstractEventLoop):
        self._loop = loop
        self._closed = self._loop.create_future()

    def close(self, reason: str):
        if not self._closed.done():
            self._closed.set_exception(SystemExit(reason))

    async def input(self, prompt: str = "") -> str:
        thread = asyncio.to_thread(lambda: input(prompt))
        tasks = [self._closed, thread]
        for result in asyncio.as_completed(tasks):
            try:
                return await result
            except:
                sys.stdin.close()
                raise
        return ""  # Some error on input

    async def print(self, *strs: str, sep: str = " ", end: str = "\n") -> None:
        thread = asyncio.to_thread(lambda: print(*strs, sep=sep, end=end))
        tasks = [self._closed, thread]
        for result in asyncio.as_completed(tasks):
            return await result


async def eval_expr(expr, locals) -> object:
    # Patch line to be able to use await inside by making it an
    # (async) generator
    line = f"((\n{expr}\n) for _ in '_')"
    expr = eval(line, locals, locals)
    if isinstance(expr, AsyncGenerator):
        expr = await expr.__anext__()
    else:
        expr = next(expr)
    if asyncio.iscoroutine(expr):
        expr = await expr
    return expr


@dataclass
class Command:
    """
    One portion of a script that can be evaluated and checked against
    expected results
    """

    prefix: str = ""
    input: str = ""
    expected: str = ""
    exception: str | None = None
    stdin: str = ""
    stdout: str = ""
    stderr: str = ""

    async def eval(self, locals) -> str:
        """
        Evaluate a command, returning a non-empty string describing any
        errors (or the empty string if it behaved as expected).
        """
        if not self.input:
            return ""

        self.input = self.input.removesuffix("\n")
        self.expected = self.expected.removesuffix("\n")
        if self.exception is not None:
            self.exception = self.exception.removesuffix("\n")

        errors = ""
        exc = None
        ret = None

        stdin, stdout, stderr = (sys.stdin, sys.stdout, sys.stderr)
        inIO, outIO, errIO = io.StringIO(self.stdin), io.StringIO(), io.StringIO()
        (sys.stdin, sys.stdout, sys.stderr) = inIO, outIO, errIO
        try:
            ret = str(await eval_expr(self.input, locals))
        except Exception as e:
            exc = e
        finally:
            outIO.flush()
            errIO.flush()
            (sys.stdin, sys.stdout, sys.stderr) = stdin, stdout, stderr

        for name, expected, got in [
            ("exception", self.exception, exc),
            ("return", self.expected, ret),
            ("stdout", self.stdout, outIO.getvalue()),
            ("stderr", self.stderr, errIO.getvalue()),
        ]:
            if expected != got:
                errors += f"Was expecting {name} {expected!r} but got {got!r}\n"

        unconsumed = inIO.getvalue()[inIO.tell() :]
        if unconsumed:
            unconsumed = unconsumed
            errors += f"Unconsumed input: {unconsumed.replace('\n', '\n0 ')}"

        if errors:
            return (
                "Error while evaluating command\n"
                + f">{self.prefix}{self.input.replace('\n', '\n> ')}\n"
                + f"|{self.prefix}{errors.replace('\n', '\n| ')}\n"
            )
        return ""


async def script(script_file: Path, locals):
    with script_file.open("rt") as fp:
        errors = ""
        # If we start reading a new input, it indicates a new command
        readingInput = False
        command = Command()
        while line := fp.readline():
            if line.startswith(">"):
                if not readingInput:
                    errors += await command.eval(locals)
                    readingInput = True
                    command = Command()
                    match = re.match(r"^\s*", line.removeprefix(">"))
                    assert match, "Should match at least the empty string"
                    command.prefix = match[0]
                command.input += line.removeprefix(">").removeprefix(command.prefix)
            else:
                readingInput = False
            if line.startswith("<"):
                command.expected += line.removeprefix("<").removeprefix(command.prefix)
            if line.startswith("0"):
                command.stdin += line.removeprefix("0").removeprefix(command.prefix)
            if line.startswith("1"):
                command.stdout += line.removeprefix("1").removeprefix(command.prefix)
            if line.startswith("2"):
                command.stderr += line.removeprefix("2").removeprefix(command.prefix)
        errors += await command.eval(locals)
        return errors


async def repl(io, locals, debug=False):
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
            await io.print("out>", await eval_expr(line, locals))
        except Exception as e:
            await io.print(traceback.format_exc())
            if debug:
                pdb.post_mortem(e.__traceback__)
