"""
Compare two different mapfiles, outputting a markdown or an SVG summary
"""

import typing as t
from parser import Area, InputSection, MapFile, OutputSection, Section
from pathlib import Path
from random import Random

import utils
from svg import svg


def shorten_with_tooltip(input_section, output_section):
    """
    Shortens the section name with a tooltip of the full name, if
    necessary.
    """
    name = input_section.pretty_name(output_section)
    short_name = utils.ellipsise_templates(name)
    if name != short_name:
        name = f'[{short_name}](# "{name}")'
    return name


def compare(old_mapfile: MapFile, new_mapfile: MapFile, md_out: t.TextIO, svg_out: t.TextIO):
    grew = set[Area | Section]()
    new = set[Area | Section]()
    for (old_area, old_sections), (new_area, new_sections) in zip(
        old_mapfile.iter_area_sections(), new_mapfile.iter_area_sections()
    ):
        old_area_size = 0
        new_area_size = 0
        old_out_secs: dict[str, OutputSection] = {}
        for sec in old_sections:
            old_area_size += sec.size
            old_out_secs[sec.name] = sec
        for new_out_sec in new_sections:
            new_area_size += new_out_sec.size
            md_out.write("\n")
            pretty = new_out_sec.pretty_name()
            if new_out_sec.name not in old_out_secs:
                size = utils.binary_prefix(new_out_sec.size)
                md_out.write(f"Added {size}B due to section {pretty}\n")
                new.add(new_out_sec)
                continue
            old_out_sec = old_out_secs.pop(new_out_sec.name)
            size_increase = new_out_sec.size - old_out_sec.size
            abs_change = utils.binary_prefix(abs(size_increase))
            if size_increase == 0:
                continue
            elif size_increase < 0:
                md_out.write(f"Shrunk by {abs_change}B from section {pretty}\n")
            else:  # old_out_sec.size < new_out_sec.size
                grew.add(new_out_sec)
                md_out.write(f"Grew by {abs_change}B from section {pretty}\n")

            old_in_secs = dict[str, InputSection](
                # Note: object names are expected to be the same
                # but the build directory might be different. Object
                # name as part of key because e.g. multiple objects
                # might have a .bss input section.
                (Path(sec.object).name + " " + sec.name, sec)
                for sec in old_out_sec.inputs
            )
            for new_in_sec in new_out_sec.inputs:
                key = Path(new_in_sec.object).name + " " + new_in_sec.name
                pretty = shorten_with_tooltip(new_in_sec, new_out_sec)
                if key not in old_in_secs:
                    size = utils.binary_prefix(new_in_sec.size)
                    md_out.write(f"- Added {size}B due to input {pretty}\n")
                    new.add(new_in_sec)
                    continue
                old_in_sec = old_in_secs.pop(key)
                if old_in_sec.size == new_in_sec.size:
                    continue
                elif old_in_sec.size > new_in_sec.size:
                    size = utils.binary_prefix(old_in_sec.size - new_in_sec.size)
                    md_out.write(f"- Shrunk by {size}B from input {pretty}\n")
                else:  # old_in_sec.size < new_in_sec.size
                    size = utils.binary_prefix(new_in_sec.size - old_in_sec.size)
                    md_out.write(f"- Grew by {size}B from input {pretty}\n")
                    grew.add(new_in_sec)
            for removed in old_in_secs.values():
                pretty = shorten_with_tooltip(removed, new_out_sec)
                size = utils.binary_prefix(removed.size)
                md_out.write(f"- Removed {size}B due to input {pretty}\n")
        for removed in old_out_secs.values():
            pretty = removed.pretty_name()
            size = utils.binary_prefix(removed.size)
            md_out.write(f"- Removed {size}B due to section {pretty}\n")
        if old_area_size < new_area_size:
            grew.add(new_area)

    random = Random(1234)

    def palette(entry: Area | Section) -> str:
        r = random.randint(0, 0x20)
        g = random.randint(0, 0x20)
        b = random.randint(0, 0x80)
        if entry in grew:
            g += 0x7F
        elif entry in new:
            g += 0xBF
        return f"#{r:02X}{g:02X}{b:02X}"

    svg(new_mapfile, palette, svg_out)
