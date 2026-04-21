from __future__ import annotations

import io
import time
import tkinter as tk
import typing as t
from asyncio import AbstractEventLoop

import cbor2
import janus
from matplotlib.backend_bases import key_press_handler
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg, NavigationToolbar2Tk
from matplotlib.figure import Figure

from comms_ccf.channel import Channel, Channels
from comms_ccf.events import add_event


async def plotter(
    loop: AbstractEventLoop,
    channels: Channels,
    output: t.Callable[[str, object, object], t.Awaitable[None]],
):
    channels.open_channel(Channel.Plot)

    while True:
        data = await channels.recv(Channel.Plot)
        args = []
        argsIo = io.BytesIO(data)
        try:
            # If there are any more arguments, decode them for formatting
            while True:
                args.append(cbor2.load(argsIo))
        except cbor2.CBORDecodeEOF:
            pass
        await add_event(Channel.Plot, *args)
        if len(args) == 2:
            await output(args[0], time.time(), args[1])
        elif len(args) == 3:
            await output(args[0], args[1], args[2])
        else:
            print("Ignoring bad plot data", args)


class Plot:
    def __init__(self) -> None:
        self._start = None
        self._target = None
        self._fig = Figure(figsize=(5, 4), dpi=72)
        self._data = {}
        self._q = janus.Queue()
        self._ax = self._fig.add_subplot()
        self._ax.set_xlabel("time / s")
        self._ax.set_ylabel("fan / 255")
        self._ax2 = self._ax.twinx()
        self._ax2.set_ylabel("temp / C")

    def attach(self, container: tk.Misc) -> Plot:
        self._container = container
        self._canvas = FigureCanvasTkAgg(self._fig, master=container)
        self._canvas.draw()

        # pack_toolbar=False will make it easier to use a layout manager later on.
        toolbar = NavigationToolbar2Tk(
            self._canvas, self._container, pack_toolbar=False
        )
        toolbar.update()

        self._canvas.mpl_connect("key_press_event", key_press_handler)

        # Packing order is important. Widgets are processed sequentially and if there
        # is no space left, because the window is too small, they are not displayed.
        # The canvas is rather flexible in its size, so we pack it last which makes
        # sure the UI controls are displayed as long as possible.
        toolbar.pack(side=tk.BOTTOM, fill=tk.X)
        self._canvas.get_tk_widget().pack(side=tk.TOP, fill=tk.BOTH, expand=True)

        self._poll()
        return self

    def _poll(self) -> None:
        try:
            val = self._q.sync_q.get_nowait()
            # Poll until queue is empty to appear responsive
            self._container.after(0, self._poll)
            if isinstance(val, tuple) and len(val) == 3:
                self._handle(*val)
        except janus.SyncQueueEmpty:
            self._container.after(100, self._poll)

    def _handle(self, cmd, args, kwargs) -> None:
        if cmd == "add":
            self._add(*args, **kwargs)
        if cmd == "clear":
            self._clear(*args, **kwargs)

    def _add(self, key, x, y) -> None:
        if self._start is None:
            self._start = x
        x -= self._start
        entry = self._data.get(key)
        if entry:
            line, xs, ys = entry
            xs.append(x)
            ys.append(y)
            line.set_data(xs, ys)
            if not line.axes:
                if key in ["fpga", "smooth"]:
                    self._ax2.add_artist(line)
                else:
                    self._ax.add_artist(line)
            line.axes.relim()
            line.axes.autoscale_view()
            if self._start:
                if self._target and self._target.axes:
                    self._target.set_data([0, x], [40, 40])
                elif self._target:
                    self._ax2.add_artist(self._target)
                else:
                    (self._target,) = self._ax2.plot([0, x], [40, 40], label="target")
        else:
            xs = [x]
            ys = [y]
            if key in ["fpga", "smooth"]:
                (line,) = self._ax2.plot(xs, ys, label=key, linestyle="--")
            else:
                (line,) = self._ax.plot(xs, ys, label=key, linestyle="-")
            self._redraw()
        self._data[key] = (line, xs, ys)
        self._redraw()
        self._fig.canvas.draw_idle()
        self._fig.canvas.flush_events()

    def _clear(self) -> None:
        self._start = None
        for key, entry in self._data.items():
            line = entry[0]
            line.set_data([], [])
            self._data[key] = (line, [], [])
        self._ax.clear()
        self._ax2.clear()
        self._redraw()
        self._fig.canvas.draw_idle()
        self._fig.canvas.flush_events()

    def _redraw(self) -> None:
        self._ax.set_xlabel("time / s")
        self._ax.set_ylabel("fan / 255")
        self._ax2.set_ylabel("temp / C")
        self._ax2.yaxis.set_label_position("right")
        lines = []
        labels = []
        for key, entry in self._data.items():
            lines.append(entry[0])
            labels.append(key)
        self._ax.legend(lines, labels, loc="lower left")

    async def add(self, *args, **kwargs) -> None:
        await self._q.async_q.put(("add", args, kwargs))

    async def clear(self, *args, **kwargs) -> None:
        await self._q.async_q.put(("clear", args, kwargs))


if __name__ == "__main__":
    root = tk.Tk()
    con = Plot()
    con.attach(root)
    tk.mainloop()
    print("Done")
