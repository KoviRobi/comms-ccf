import asyncio
import signal
import tkinter as tk
import typing as t
import traceback

from cobs.cobs import DecodeError
import janus

from tk_console import Console
from tk_table import Table

from background import BackgroundTasks
from channel import Channels
from log import print_logs
from repl import repl
from rpc import Rpc
from transport import StreamTransport

T = t.TypeVar("T")


class AsyncConsole:
    def __init__(self) -> None:
        self._container: tk.Misc = None  # type: ignore only used after attach by internal poll
        self._console: Console = None  # type: ignore only used after attach by internal poll
        self._toTkQueue = janus.Queue()
        self._fromTkQueue = janus.Queue()

    def attach(self, container: tk.Misc) -> Console:
        self._container = container
        self._console = Console(self._container)
        self._container.after(0, self._poll)
        return self._console

    def _poll(self):
        try:
            val = self._toTkQueue.sync_q.get_nowait()
            # Poll until queue is empty to appear responsive
            self._container.after(0, self._poll)
            if isinstance(val, tuple) and len(val) == 3:
                self._handle(*val)
        except janus.SyncQueueEmpty:
            self._container.after(100, self._poll)

    def _handle(self, cmd, args, kwargs):
        if cmd == "input":
            self._container.after(0, self._respond, self._console.input(*args, **kwargs))
        elif cmd == "print":
            self._console.print(*args, **kwargs)

    def _respond(self, value):
        try:
            self._fromTkQueue.sync_q.put_nowait(value)
        except janus.SyncQueueFull:
            self._container.after(100, self._respond, value)

    async def print(self, *args, **kwargs):
        await self._toTkQueue.async_q.put(("print", args, kwargs))

    async def input(self, *args, **kwargs):
        await self._toTkQueue.async_q.put(("input", args, kwargs))
        return await self._fromTkQueue.async_q.get()


console = AsyncConsole()


def tk_loop():
    global table
    root = tk.Tk()
    root.title("Comms-CCF")

    tframe = tk.Frame(root)
    tframe.pack(expand=True, fill="both")
    table = Table(tframe, columns=("Component", "Severity", "Message"))
    table.pack_configure(fill="both", expand=True)

    cframe = tk.Frame(root)
    cframe.pack(expand=True, fill="both")
    cons = console.attach(cframe)
    cons.pack_configure(expand=True)
    root.mainloop()
    exit()


async def amain():
    global table
    loop = asyncio.get_event_loop()
    background = BackgroundTasks(loop)

    background.add(asyncio.to_thread, tk_loop)

    rx, tx = await asyncio.open_connection("localhost", 4321)
    transport = StreamTransport(rx, tx)
    channels = Channels(transport, loop)
    rpc = Rpc(channels, console)
    channels.open_channel(0)

    background.add(channels.loop)

    async def add_to_table(level: str, module: int, msg: str):
        await table.insert_row((level, module, msg))

    background.add(print_logs, channels, add_to_table)

    await rpc.discover(timeout=0.5)

    locals = {k: v for k, v in rpc.methods().items()}
    locals["help"] = rpc.help
    locals["dir"] = dir

    await console.print("Use help(name=None) for discovered methods")
    await console.print("(optionally name to document just that method)")

    await repl(console, locals)


if __name__ == "__main__":
    signal.signal(signal.SIGINT, signal.SIG_DFL)
    asyncio.run(amain())
