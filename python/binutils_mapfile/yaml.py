"""
Output the mapfile as a YAML which is suitable for diff-ing because it
doesn't include addresses.
"""

from __future__ import annotations

import sys
import typing as t
from json import dump

import binutils_mapfile.utils as utils
from binutils_mapfile.parser import MapFile, Section


def format_section_info(container_size: int, section: Section) -> str:
    return (
        "["
        + section.name
        # Don't print address to make diffs easier
        # f", 0x{section.address:08X}" +
        + f", {len(section):08X}"
        + f", {100.0 * len(section) / container_size:5.2f}"
        + "]"
    )


def diffable_format(mapfile: MapFile, file: t.TextIO = sys.stdout):
    """
    Print in a YAML parseable format
    """
    for area, area_sections in mapfile.iter_area_sections():
        dump(area.name, fp=file)
        file.write(":\n")
        for output_section in sorted(area_sections, key=len):
            file.write("  - name: ")
            dump(output_section.name, fp=file)
            file.write("\n")
            for name, value in [
                # Don't print address to make diffs easier
                # ("address:", f"0x{output_section.address:08X}"),
                ("size", f"0x{output_section.size:08X}"),
                ("fill", f"0x{output_section.fill:08X}"),
                ("length", f"0x{len(output_section):08X}"),
            ]:
                file.write("    ")
                file.write(name)
                file.write(": ")
                file.write(value)
                file.write("\n")
            file.write("    input sections:\n")
            for input_section in sorted(output_section.inputs, key=len):
                file.write("      - name: ")
                dump(input_section.name, fp=file)
                file.write("\n")
                for name, value in [
                    # Don't print address to make diffs easier
                    # ("address:", f"0x{input_section.address:08X}"),
                    ("size", f"0x{input_section.size:08X}"),
                    ("fill", f"0x{input_section.fill:08X}"),
                    ("length", f"0x{len(input_section):08X}"),
                ]:
                    file.write("        ")
                    file.write(name)
                    file.write(": ")
                    file.write(value)
                    file.write("\n")
                file.write("        symbols:\n")
                for sym in input_section.symbols:
                    file.write("          - ")
                    dump(sym.name, fp=file)
