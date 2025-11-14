#!/usr/bin/env python3

import tkinter as tk
import typing as t
from tkinter import scrolledtext


class Console(tk.Frame):
    def __init__(self, parent):
        super().__init__(parent)
        self.pack(expand=True, fill="both")

        self._output = scrolledtext.ScrolledText(self)
        self._output.pack(expand=True, fill="both")
        self._output.configure(state="normal")
        self._output.bind("<Return>", self.accept_input)
        self._output.bind("<Tab>", self.complete_input)
        self._output.bind("<Control-w>", self.erase_word)
        self._output.bind("<Control-u>", self.erase_linestart)
        self._output.bind("<Control-k>", self.erase_lineend)
        self._input = tk.StringVar()

    def append_output(self, text: object) -> None:
        if text:
            self._output.insert(tk.END, str(text))

    def print(self, *strs: str, sep: str = " ", end: str = "\n") -> None:
        print("print strs", strs)
        self.append_output(sep.join(strs) + end)

    def erase_word(self, event: tk.Event):
        widget = t.cast(tk.Text, event.widget)
        widget.delete("insert -1 c wordstart", "insert")
        return "break"

    def erase_linestart(self, event: tk.Event):
        widget = t.cast(tk.Text, event.widget)
        widget.delete("insert linestart", "insert")
        return "break"

    def erase_lineend(self, event: tk.Event):
        widget = t.cast(tk.Text, event.widget)
        widget.delete("insert", "insert lineend")
        return "break"

    def wait_input(self) -> str:
        return self._input.get()

    def input(self, prompt: str = "") -> str:
        self.print(prompt, end="")
        return self.wait_input()

    def accept_input(self, event: tk.Event):
        if event.state:
            return ""
        widget = t.cast(tk.Text, event.widget)
        data = widget.get("insert linestart", "insert lineend")
        self._input.set(data)
        return "break"

    def complete_input(self, event: tk.Event):
        if event.state:
            return ""
        widget = t.cast(tk.Text, event.widget)
        print("Complete", event, widget.get("insert linestart", "insert"), flush=True)
        return "break"

    def focus(self):
        self._output.focus()

    def test(self):
        self.append_output("Hello, world")
        self.append_output("Nice to greet you")


if __name__ == "__main__":
    root = tk.Tk()
    con = Console(root)
    con.test()
    con.focus()
    con.pack()
    tk.mainloop()
    print("Done")
