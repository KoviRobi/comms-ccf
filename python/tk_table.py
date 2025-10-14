import tkinter as tk
from tkinter import ttk
import janus


# Create a Treeview widget
class Table(ttk.Treeview):

    def __init__(self, master, columns) -> None:
        self.vbar = tk.Scrollbar(master)
        self.vbar.pack(side=tk.RIGHT, fill=tk.Y)
        super().__init__(master, columns=columns, yscrollcommand=self.set_track)
        self.track = True
        self.vbar["command"] = self.yview_track

        super().column("#0", width=0, stretch=tk.NO)
        super().heading("#0", text="")
        for col in columns:
            super().heading(col, text=col)

        # Configure alternating row colors
        self.tag_configure("oddrow", background="#E8E8E8")
        self.tag_configure("evenrow", background="#FFFFFF")

        self.queue = janus.Queue()
        self.data = []
        self._poll()

    # TODO: tracking scrollbar widget?
    def set_track(self, top, bot, *args, **kwargs):
        print("set", top, bot, args, kwargs)
        track = float(bot) >= 1.0
        if self.track:
            self.yview_scroll(1, tk.PAGES)
            self.track = True
        else:
            self.track = track
        self.vbar.set(top, bot)

    def yview_track(self, *args, **kwargs):
        if args[0] == "moveto":
            self.track = float(args[1]) >= 1.0
        elif args[0] == "scroll" and args[2] == tk.PAGES:
            self.track = args[1] == "1"
        elif args[0] == "scroll" and args[2] == tk.UNITS:
            self.track = self.track and args[1] == "1"
        print("yview", args, kwargs)
        self.yview(*args, **kwargs)

    def _poll(self):
        try:
            val = self.queue.sync_q.get_nowait()
            # Poll until queue is empty to appear responsive
            self.master.after(0, self._poll)
            if isinstance(val, tuple) and len(val) == 3:
                self._handle(*val)
        except janus.SyncQueueEmpty:
            self.master.after(100, self._poll)

    def _handle(self, cmd, args, kwargs):
        if cmd == "insert_row":
            self._insert_row(*args, **kwargs)

    def _insert_row(self, row, index: int = -1):
        if index < 0:
            index = len(self.data) + 1 - index
            self.data.insert(index, row)
        if index % 2 == 0:
            super().insert(parent="", index=index, values=row, tags=("evenrow",))
        else:
            super().insert(parent="", index=index, values=row, tags=("oddrow",))

    async def insert_row(self, *args, **kwargs):
        await self.queue.async_q.put(("insert_row", args, kwargs))
