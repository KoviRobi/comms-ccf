"""
Miscellaneous utils used for the binutils mapfile parser.
"""

from __future__ import annotations

import re
import typing as t


def simplify_std_templates(symbol: str):
    for char in [
        ("char", ""),
        ("wchar_t", "w"),
        ("char8_t", "u8"),
        ("char16_t", "u16"),
        ("char32_t", "u32"),
    ]:
        for template in [
            (
                "basic_string_view<{0}, std::char_traits<{0}> >",
                "{0}string_view",
            ),
            (
                "basic_string<{0}, std::char_traits<{0}>, std::allocator<{0}> >",
                "{0}string",
            ),
        ]:
            symbol = symbol.replace(
                template[0].format(char[0]), template[1].format(char[1])
            )
    return symbol


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


class IterWithPutBack[T]:
    """
    Iterator that allows you to put elements back if you decide you
    didn't want to consume them
    """

    def __init__(self, iter: t.Iterator[T]):
        self._iter = iter
        self._front: list[T] = []

    def __iter__(self) -> t.Iterator[T]:
        return self

    def __next__(self) -> T:
        if self._front:
            return self._front.pop()
        return next(self._iter)

    def put_back(self, elem: T):
        self._front.append(elem)

    def peek(self) -> T:
        if not self._front:
            self._front.append(next(self._iter))
        return self._front[-1]
