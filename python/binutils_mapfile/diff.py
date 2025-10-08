"""
Compare two different mapfiles, outputting a markdown or an SVG summary
"""

from dataclasses import dataclass, field
import typing as t
from parser import Area, InputSection, MapFile, OutputSection, Section
from pathlib import Path
from random import Random

import utils
from svg import svg as write_svg


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


@dataclass
class Diff:
    grew: dict[Area | Section, int] = field(default_factory=dict)
    shrunk: dict[Area | Section, int] = field(default_factory=dict)
    added: dict[Area | Section, int] = field(default_factory=dict)
    removed: dict[Area | Section, int] = field(default_factory=dict)


def compare(old_mapfile: MapFile, new_mapfile: MapFile) -> Diff:
    diff = Diff()
    for (old_area, old_sections), (new_area, new_sections) in zip(
        old_mapfile.iter_area_sections(), new_mapfile.iter_area_sections()
    ):
        old_area_size = 0
        new_area_size = 0
        old_out_secs: dict[str, OutputSection] = {}
        for sec in old_sections:
            old_area_size += len(sec)
            old_out_secs[sec.name] = sec
        for new_out_sec in new_sections:
            new_area_size += len(new_out_sec)
            md_out.write("\n")
            pretty = new_out_sec.pretty_name()
            if new_out_sec.name not in old_out_secs:
                size = utils.binary_prefix(new_out_sec.size)
                diff.added[new_out_sec] = new_out_sec.size
                continue
            old_out_sec = old_out_secs.pop(new_out_sec.name)
            size_increase = len(new_out_sec) - len(old_out_sec)
            if size_increase == 0:
                continue
            elif size_increase < 0:
                diff.shrunk[new_out_sec] = -size_increase
            else:  # size_increase > 0
                diff.grew[new_out_sec] = size_increase

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
                    diff.added[new_in_sec] = new_in_sec.size
                    continue
                old_in_sec = old_in_secs.pop(key)
                size_increase = len(new_in_sec) - len(old_in_sec)
                if size_increase == 0:
                    continue
                elif size_increase < 0:
                    diff.shrunk[new_in_sec] = -size_increase
                else:  # size_increase > 0
                    diff.grew[new_in_sec] = size_increase
            for removed in old_in_secs.values():
                diff.removed[removed] = removed.size
        for removed in old_out_secs.values():
            diff.removed[removed] = removed.size
        size_increase = len(new_area) - len(old_area)
        if size_increase < 0:
            diff.shrunk[new_area] = size_increase
        elif size_increase > 0:
            diff.grew[new_area] = size_increase

    return diff

def md(diff: Diff, md_out: t.TextIO):
        md_out.write(f"Added {size}B due to section {pretty}\n")
        md_out.write(f"Shrunk by {abs_change}B from section {pretty}\n")
        md_out.write(f"Grew by {abs_change}B from section {pretty}\n")
            md_out.write(f"- Added {size}B due to input {pretty}\n")
            md_out.write(f"- Shrunk by {size}B from input {pretty}\n")
            md_out.write(f"- Grew by {size}B from input {pretty}\n")
        md_out.write(f"- Removed {size}B due to input {pretty}\n")
    md_out.write(f"- Removed {size}B due to section {pretty}\n")

def svg(mapfile: MapFile, diff: Diff, svg_out: t.TextIO):
    random = Random(1234)

    def palette(entry: Area | Section) -> str:
        r = random.randint(0, 0x20)
        g = random.randint(0, 0x20)
        b = random.randint(0, 0x80)
        if entry in diff.grew:
            g += 0x7F
        elif entry in diff.added:
            g += 0xBF
        return f"#{r:02X}{g:02X}{b:02X}"

    write_svg(mapfile, palette, svg_out)
