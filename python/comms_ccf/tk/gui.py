import tkinter as tk
import typing as t

import janus

from comms_ccf.tk.con import Console
from comms_ccf.tk.table import Table


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
            self._container.after(
                0, self._respond, self._console.input(*args, **kwargs)
            )
        elif cmd == "print":
            self._console.print(*args, **kwargs)

    def _respond(self, value):
        try:
            self._fromTkQueue.sync_q.put_nowait(value)
        except janus.SyncQueueFull:
            self._container.after(100, self._respond, value)

    async def print(self, *strs: str, sep: str = " ", end: str = "\n") -> None:
        try:
            await self._toTkQueue.async_q.put(("print", strs, {"sep": sep, "end": end}))
        except janus.AsyncQueueShutDown:
            pass

    async def input(self, prompt: str = "") -> str:
        try:
            await self._toTkQueue.async_q.put(("input", (prompt,), {}))
            return await self._fromTkQueue.async_q.get()
        except janus.AsyncQueueShutDown:
            pass
        assert False

    def close(self, reason: str):
        self._fromTkQueue.close()
        self._toTkQueue.close()
        self._container.quit()
        raise SystemExit(reason)


class TkGui:
    console: AsyncConsole | None
    logs: Table | None

    def __init__(self, logs: bool, console: bool) -> None:
        self.console = None
        self.logs = None
        if console:
            self.console = AsyncConsole()
        if logs:
            self.logs = Table("Severity", "Component", "Message")

    def loop(self) -> None:
        root = tk.Tk()
        root.title("Comms-CCF")

        if self.logs is not None:
            tframe = tk.Frame(root)
            tframe.pack(expand=True, fill="both")
            logs_element = self.logs.attach(tframe)
            logs_element.pack_configure(fill="both", expand=True)

        if self.console is not None:
            cframe = tk.Frame(root)
            cframe.pack(expand=True, fill="both")
            console_element = self.console.attach(cframe)
            console_element.pack_configure(expand=True)
        root.mainloop()
        if self.console is not None:
            self.console.close("GUI closed")
