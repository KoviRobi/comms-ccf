from __future__ import annotations

import inspect
import re
import sys
import typing as t


def indent(
    *args: str,
    level: int = 1,
    file: t.TextIO = sys.stdout,
    strip: int = 0,
    tabstop: int = 8,
):
    """
    Print an indent corresponding to the current python indent.
    If args is specified, it writes each to the file, and then a newline.
    """
    frame = inspect.currentframe()
    context = None
    match = None
    while frame is not None and level > 0:
        frame = frame.f_back
        level -= 1
    if frame is not None:
        context = inspect.getframeinfo(frame).code_context
    if context is not None and len(context) > 0:
        match = re.match(r"^\s*", context[0])
    if match is not None:
        file.write(match[0].replace("\t", " " * tabstop)[strip:])
    # No indent
    if args != []:
        sep = ""
        for arg in args:
            file.write(sep)
            file.write(arg)
            sep = " "
        file.write("\n")


def ellipsise_templates(symbol: str):
    """
    Shorten templated values by replacing them with ellipsis (...)

    Assumes parenthesised templates containing expressions,
    e.g. `Foo<(1 < 2)>`
    """
    innermost_angle_expr = re.compile(r"(.*)<\(.*?\)>(.*)")
    while m := innermost_angle_expr.match(symbol):
        symbol = m[1] + "⟨(...)⟩" + m[2]
    innermost_angle = re.compile(r"(.*)<[^>]*>(.*)")
    while m := innermost_angle.match(symbol):
        symbol = m[1] + "⟨...⟩" + m[2]
    return symbol.replace("⟨", "<").replace("⟩", ">")


def xmlescape(s: str) -> str:
    return (
        s.replace("&", "&amp;")
        .replace("<", "&lt;")
        .replace(">", "&gt;")
        .replace("'", "&apos;")
        .replace('"', "&quot;")
    )


def binary_prefix(number: int | float) -> str:
    """Returns the Ki/Mi/Gi/etc prefixed version of the given number"""
    magnitude = 0
    while int(number) > 1024:
        number = number / 1024
        magnitude += 1
    prefixes = ["", "Ki", "Mi", "Gi", "Ti", "Pi", "Ei", "Zi", "Yi", "Ri", "Qi"]
    prefix = prefixes[min(magnitude, len(prefixes) - 1)]
    if isinstance(number, int):
        return f"{number}{prefix}"
    else:
        return f"{number:.2f}{prefix}"
