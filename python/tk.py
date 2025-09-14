import math
import ctypes
import asyncio

import tkinter as tk
from tkinter import ttk
from _tkinter import DONT_WAIT, getbusywaitinterval, TCL_VERSION, TK_VERSION
from TkConsole import Console

from channel import Channels
from repl import repl
from rpc import Rpc
from transport import StreamTransport

background_tasks = set()

_tcl = ctypes.CDLL(f"libtcl{TCL_VERSION}.so")
_tk = ctypes.CDLL(f"libtk{TK_VERSION}.so")

getnummainwindows = _tk["Tk_GetNumMainWindows"]

setnotifier = _tcl["Tcl_SetNotifier"]

root = tk.Tk()
root.title("Comms-CCF")
console = Console(root)

# TODO: Use aiotkinter on platforms that support it?
async def tk_loop():
    while True:
        if getnummainwindows() == 0:
            break
        if root.dooneevent(DONT_WAIT) == 0:
            await asyncio.sleep(getbusywaitinterval() / 1000.0)

async def amain():
    tk_task = asyncio.create_task(tk_loop())
    background_tasks.add(tk_task)
    tk_task.add_done_callback(background_tasks.discard)

    rx, tx = await asyncio.open_connection("localhost", 4321)
    transport = StreamTransport(rx, tx)
    loop = asyncio.get_event_loop()
    channels = Channels(transport, loop)
    rpc = Rpc(channels)
    channels.open_channel(0)
    channels.open_channel(1)

    channel_task = loop.create_task(channels.loop())
    background_tasks.add(channel_task)
    channel_task.add_done_callback(background_tasks.discard)

    log = await channels.recv(1, timeout=math.inf)
    console.print(log.decode("utf-8", "ignore"))

    await rpc.discover()

    locals = {k: v for k, v in rpc.methods().items()}
    locals["help"] = rpc.help
    locals["dir"] = dir

    console.print("Use help(name=None) for discovered methods")
    console.print("(optionally name to document just that method)")

    await repl(console, locals)

if __name__ == "__main__":
    asyncio.run(amain())
