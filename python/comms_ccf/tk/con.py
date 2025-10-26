#!/usr/bin/env python3

import io
import sys
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

    def append_output(self, text: object) -> None:
        if text:
            self._output.insert(tk.END, str(text))

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

    def accept_input(self, event: tk.Event):
        if event.state:
            return ""
        widget = t.cast(tk.Text, event.widget)
        data = widget.get("insert linestart", "insert lineend")
        exc = None
        output = None
        inp, out, err = sys.stdin, sys.stdout, sys.stderr
        try:
            sys.stdin, sys.stdout, sys.stderr = (
                io.StringIO(),
                io.StringIO(),
                io.StringIO(),
            )
            output = eval(data)
        except BaseException as e:
            exc = e
        finally:
            sys.stdin, sys.stdout, sys.stderr, inp, out, err = (
                inp,
                out,
                err,
                sys.stdin,
                sys.stdout,
                sys.stderr,
            )
        self.append_output(out.getvalue())
        self.append_output(err.getvalue())
        self.append_output(exc)
        self.append_output(output)
        return "break"

    def complete_input(self, event: tk.Event):
        if event.state:
            return ""
        widget = t.cast(tk.Text, event.widget)
        pos = widget.index(tk.INSERT)
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
