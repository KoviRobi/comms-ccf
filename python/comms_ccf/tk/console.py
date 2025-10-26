"""
TkConsole: A Custom Console Widget for Tkinter

This module provides a custom console widget that offers a terminal-like interface
using the Tkinter library. It allows users to display text output, take user input,
and perform copy/paste actions within the console.

Classes:
    Console: A class representing the custom console widget.
"""

import dataclasses
import tkinter as tk
import tkinter.font as tkFont
from tkinter import scrolledtext
from typing import Union


class Console(tk.Frame):
    """
    A custom console widget that provides a terminal-like interface using Tkinter.

    Args:
        parent_ (tk.Tk): The parent Tkinter window.
        **kwargs: Additional keyword arguments for configuring the console's appearance.

    Attributes:
        font (tkFont.Font): The font used for the console's text.
        background (str): The background color of the console.
        foreground (str): The foreground color (text color) of the console.
        text_area (scrolledtext.ScrolledText): The text area of the console for displaying output.
        entry (tk.Entry): The entry field for user input.
        edit_menu (tk.Menu): The context menu for copy/paste actions.
        helpers (Helpers): An inner class containing helper attributes and methods.
    """

    @dataclasses.dataclass
    class Helpers:
        """
        A helper class containing various attributes for assisting the Console class.

        Attributes:
            user_input_var (tk.StringVar or None): A variable to hold user input.
            entry_x (int or None): X-coordinate of the entry field.
            text_cursor_position (int or None): The position of the text cursor in the entry field.
            yview (float or None): The vertical scroll position of the text area.
            index (float or None): The current line index of the text cursor in the text area.
        """

        user_input_var: Union[tk.StringVar, None] = None
        entry_x: Union[int, None] = None
        text_cursor_position: Union[int, None] = None
        yview: Union[float, None] = None
        index: Union[float, None] = None

    def __init__(self, parent_, **kwargs):
        """
        Initialize the Console widget with the provided parent Tkinter window and appearance settings.

        Args:
            parent_ (tk.Tk): The parent Tkinter window.
            **kwargs: Additional keyword arguments for configuring the console's appearance.

        Attributes:
            font (tkFont.Font): The font used for the console's text.
            background (str): The background color of the console.
            foreground (str): The foreground color (text color) of the console.
            text_area (scrolledtext.ScrolledText): The text area of the console for displaying output.
            entry (tk.Entry): The entry field for user input.
            edit_menu (tk.Menu): The context menu for copy/paste actions.
            helpers (Helpers): An inner class containing helper attributes and methods.
        """
        tk.Frame.__init__(self, parent_)
        self.parent = parent_
        self.font = kwargs.get("font", tkFont.Font(family="Monospace", size=10))
        self.bold = tkFont.Font(**{**self.font.actual(), "weight": tkFont.BOLD})
        self.background = kwargs.get("background", "#232627")
        self.foreground = kwargs.get("foreground", "#FFFFFF")
        self.text_area = scrolledtext.ScrolledText(
            self.parent,
            wrap=tk.WORD,
            font=self.font,
            background=self.background,
            foreground=self.foreground,
            padx=0,
            pady=0,
            borderwidth=0,
            border=0.0,
            insertborderwidth=0,
            selectborderwidth=0,
        )
        self.text_area.pack(expand=True, fill="both", padx=0, pady=0, ipady=0, ipadx=0)
        self.text_area.place(relwidth=1, relheight=1)
        self.text_area.tag_configure("bold", {"font": tkFont.BOLD})
        self.entry = None
        self.edit_menu = tk.Menu(self.parent, tearoff=0)
        self.parent.bind("<Button-3>", self.show_edit_menu)
        self.parent.bind("<Control-Shift-c>", self.copy_text)
        self.parent.bind("<Control-Shift-v>", self.paste_text)
        self.parent.bind("<Control-Shift-C>", self.copy_text)
        self.parent.bind("<Control-Shift-V>", self.paste_text)
        self.text_area.bind("<Configure>", self.adjust_on_configure)
        self.parent.bind("<Home>", self.go_to_home)
        self.parent.bind("<End>", self.go_to_end)
        self.parent.bind("<Prior>", self.page_up)
        self.parent.bind("<Next>", self.page_down)
        self.helpers = self.Helpers()

    def go_to_home(self, event):
        """Scrolls the text area to the beginning."""
        self.text_area.see("1.0")

    def go_to_end(self, event):
        """Scrolls the text area to the end."""
        self.text_area.see(tk.END)

    def page_up(self, event):
        """Scrolls the text area up by one page."""
        self.text_area.yview_scroll(-1, tk.PAGES)

    def page_down(self, event):
        """Scrolls the text area down by one page."""
        self.text_area.yview_scroll(1, tk.PAGES)

    def adjust_on_configure(self, event):
        """
        Adjusts the console's components when the window is resized.
        This method updates the entry field's width and ensures that the text area remains scrolled to the end.
        """
        self.parent.update_idletasks()
        if self.entry:
            if not self.helpers.entry_x:
                self.helpers.entry_x = self.entry.winfo_x()
            self.entry.configure(
                width=(self.text_area.winfo_width() - self.helpers.entry_x - 10)
                // self.font.measure("0")
            )
        if not self.helpers.yview and not self.helpers.index:
            self.helpers.yview = round(self.text_area.yview()[1], 1)
            self.helpers.index = float(self.text_area.index(tk.CURRENT)) // (
                self.text_area.winfo_height() // self.font.measure("0")
            )
        if self.helpers.yview < self.helpers.index or self.helpers.yview == 1.0:
            self.text_area.see(tk.END)
        self.helpers.yview = None
        self.helpers.index = None

    def write_output(self, output):
        """
        Writes the given output text to the text area and ensures that it's visible.

        Args:
            output (str): The text to be displayed in the console.
        """
        import re

        # double-struck should be bold
        doublestruck = re.split("(.\b.)", output)
        for text in doublestruck:
            tag = ""
            if len(text) == 3 and text[0] == text[2] and text[1] == "\b":
                text = text[0]
                tag = "bold"
            self.parent.update_idletasks()
            yview = round(self.text_area.yview()[1], 1)
            index = float(self.text_area.index(tk.CURRENT)) // (
                self.text_area.winfo_height() // self.font.measure("0")
            )
            self.text_area.configure(state="normal")
            self.text_area.insert(tk.END, text, tag)
            self.text_area.configure(state="disabled")
            if yview < index or yview == 1.0:
                self.text_area.see(tk.END)

    def stop(self):
        """Destroys the parent window, effectively closing the console."""
        self.parent.destroy()

    def print(self, *args, **kwargs):
        """
        Prints the provided text in the console's text area.

        Args:
            *args: Variable number of arguments to be concatenated and printed.
            **kwargs: Optional keyword arguments for controlling the print behavior.
        """
        sep = kwargs.get("sep", " ")
        end = kwargs.get("end", "\n")
        text = sep.join(map(str, args))
        self.write_output(text + end)

    def show_edit_menu(self, event):
        """Shows the context menu (right-click menu) for copy/paste actions."""
        self.edit_menu.delete(0, tk.END)
        clicked_widget = self.parent.winfo_containing(event.x_root, event.y_root)
        if clicked_widget == self.text_area and self.text_area.tag_ranges(tk.SEL):
            self.edit_menu.add_command(
                label="Copy", command=self.copy_text, accelerator="Ctrl+Shift+C"
            )
        if clicked_widget == self.entry:
            self.edit_menu.add_command(
                label="Paste", command=self.paste_text, accelerator="Ctrl+Shift+V"
            )
        if self.edit_menu.index("end") != -1:
            self.edit_menu.post(event.x_root, event.y_root)
            self.parent.bind("<Button-1>", lambda event: self.edit_menu.unpost())

    def copy_text(self, event=None):
        """Copies the selected text to the clipboard."""
        selected_text = self.text_area.get(tk.SEL_FIRST, tk.SEL_LAST)
        if selected_text:
            self.parent.clipboard_clear()
            self.parent.clipboard_append(selected_text)

    def paste_text(self, event=None):
        """Pastes text from the clipboard into the entry field."""
        clipboard_text = self.parent.clipboard_get()
        if clipboard_text:
            self.entry.insert(tk.INSERT, clipboard_text)

    def input(self, prompt=""):
        """
        Takes user input via the entry field and returns the entered text.

        Args:
            prompt (str, optional): A prompt message to be displayed before input.

        Returns:
            str: The user-entered text.
        """

        def callback():
            """
            Callback function to track the text cursor position in the entry field.
            """
            self.helpers.text_cursor_position = self.entry.index(tk.INSERT)

        def on_enter(event=None):
            """
            Callback function to handle the 'Return' or 'Enter' key press event.
            """
            self.print(self.helpers.user_input_var.get())
            self.entry.destroy()
            self.parent.unbind("<Button-1>")
            self.entry = None

        def prevent_text_cursor_movement(event=None):
            """
            Prevent text cursor movement in the entry field.
            """
            self.entry.icursor(self.helpers.text_cursor_position)

        if prompt:
            self.print(prompt, end="")
        self.parent.update_idletasks()
        yview = round(self.text_area.yview()[1], 1)
        index = float(self.text_area.index(tk.CURRENT)) // (
            self.text_area.winfo_height() // self.font.measure("0")
        )
        self.helpers.user_input_var = tk.StringVar()
        self.entry = tk.Entry(
            self.text_area,
            textvariable=self.helpers.user_input_var,
            font=self.font,
            relief=tk.FLAT,
            borderwidth=0,
            border=0.0,
            insertborderwidth=0,
            selectborderwidth=0,
            highlightthickness=0,
            background=self.background,
            foreground=self.foreground,
            insertbackground=self.foreground,
            insertwidth=1,
            insertofftime=0,
        )
        self.entry.pack(
            side=tk.LEFT, fill="x", padx=0, pady=0, ipady=0, ipadx=0, expand=True
        )
        self.text_area.window_create(
            tk.INSERT, padx=0, pady=0, stretch=1, window=self.entry
        )
        if yview < index or yview == 1.0:
            self.text_area.see(tk.END)
        self.adjust_on_configure(None)
        self.helpers.user_input_var.trace_add(
            "write", lambda name, index, mode: callback()
        )
        self.entry.bind("<Return>", on_enter)
        self.entry.bind("<KP_Enter>", on_enter)
        self.parent.bind("<Button-1>", prevent_text_cursor_movement)
        self.entry.focus_set()
        self.parent.wait_window(self.entry)
        user_input_ = self.helpers.user_input_var.get()
        return user_input_
