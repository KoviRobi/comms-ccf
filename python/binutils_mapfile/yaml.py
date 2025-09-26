from __future__ import annotations

import sys
import typing as t
from json import dumps
from parser import MapFile, Section

import utils


def format_section_info(container_size: int, section: Section) -> str:
    return (
        "["
        + section.name
        # Don't print address to make diffs easier
        # f", 0x{section.address:08X}" +
        + f", {section.size:08X}"
        + f", {100.0 * section.size / container_size:5.2f}"
        + "]"
    )


def diffable_format(mapfile: MapFile, file: t.TextIO = sys.stdout):
    """
    Print in a YAML parseable format
    """

    def indent(*args):
        return utils.indent(*args, level=2, file=file, strip=8)

    for area, area_sections in mapfile.iter_area_sections():
        indent(dumps(area.name) + ":")
        for output_section in sorted(area_sections, key=lambda sec: sec.size):
            indent("- name:", dumps(output_section.name))
            # Don't print address to make diffs easier
            # indent(" ", "address:", f"0x{output_section.address:08X}")
            indent(" ", "size:", f"0x{output_section.size:08X}")
            indent(" ", "input sections:")
            for input_section in sorted(
                output_section.inputs, key=lambda sec: sec.size
            ):
                indent("-", "name:", dumps(input_section.name))
                # Don't print address to make diffs easier
                # indent(" ", "address:", f"0x{input_section.address:08X}")
                indent(" ", "size:", f"0x{input_section.size:08X}")
                indent(" ", "symbols:")
                for sym in input_section.symbols:
                    indent("-", dumps(sym.name))
