#!/usr/bin/env python3

import re
import sys

ESCAPE_RE = re.compile(r"\[NON-XML-CHAR-0x1B\]\[(\d+)m")


def csi_escape(match: re.Match[str]):
    if match[0] == "":
        n = 0
    else:
        n = int(match[1], 16)
    if n == 0:
        return "</span>"
    colors = {
        0: ("darkgray", "lightgray"),
        1: ("red", "lightred"),
        2: ("green", "lightgreen"),
        3: ("yellow", "lightyellow"),
        4: ("blue", "lightblue"),
        5: ("magenta", "lightmagenta"),
        6: ("cyan", "lightcyan"),
        7: ("gray", "white"),
    }
    if n & 0x40:
        style = "background"
    else:
        style = "color"
    if (n & 0x0F) in colors:
        if n & 0x80:
            color = colors[n & 0x0F][0]
        else:
            color = colors[n & 0x0F][1]
    return f'<span style="{style}: {color};">'


try:
    with open("test.junit") as fp:
        while line := sys.stdin.readline():
            print(ESCAPE_RE.sub(csi_escape, line), end="")
except EOFError:
    pass
