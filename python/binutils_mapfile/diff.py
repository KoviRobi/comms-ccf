"""
Compare two different mapfiles, outputting a markdown or an SVG summary
"""

import typing as t
from enum import StrEnum, auto
from itertools import groupby
from parser import Area, InputSection, MapFile, OutputSection, Section
from pathlib import Path
from random import Random

import utils
from svg import svg as write_svg


def shorten_with_details(section_or_area, parent_prefix: str = ""):
    """
    Shortens the section name with a tooltip of the full name, if
    necessary.
    """
    name = section_or_area.pretty_name(parent_prefix)
    short_name = utils.xmlescape(utils.ellipsise_templates(name))
    name = utils.xmlescape(name)
    if name != short_name:
        name = f'<details style="display:inline-block;"><summary><a>{short_name}</a></summary>{name}</details>'
    return name

class Change(StrEnum):
    Add = auto()
    Remove = auto()
    Increase = auto()
    Decrease = auto()

Diff = list[tuple[Area | OutputSection | InputSection, Change, int]]

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
            if new_out_sec.name not in old_out_secs:
                diff.append((new_out_sec, Change.Add, len(new_out_sec)))
                continue
            old_out_sec = old_out_secs.pop(new_out_sec.name)
            size_increase = len(new_out_sec) - len(old_out_sec)
            if size_increase == 0:
                continue
            elif size_increase < 0:
                diff.append((new_out_sec, Change.Decrease, -size_increase))
            else:  # size_increase > 0
                diff.append((new_out_sec, Change.Increase, size_increase))

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
                if key not in old_in_secs:
                    diff.append((new_in_sec, Change.Add, len(new_in_sec)))
                    continue
                old_in_sec = old_in_secs.pop(key)
                size_increase = len(new_in_sec) - len(old_in_sec)
                if len(old_in_sec) == len(new_in_sec):
                    continue
                elif size_increase < 0:
                    diff.append((new_in_sec, Change.Decrease, -size_increase))
                else:  # size_increase > 0
                    diff.append((new_in_sec, Change.Increase, size_increase))
            for removed in old_in_secs.values():
                diff.append((removed, Change.Remove, len(removed)))
        for removed in old_out_secs.values():
            diff.append((removed, Change.Remove, len(removed)))
        size_increase = len(new_area) - len(old_area)
        if size_increase < 0:
            diff.append((new_area, Change.Decrease, size_increase))
        elif size_increase > 0:
            diff.append((new_area, Change.Increase, size_increase))

    return diff

def md(diff: Diff, md_out: t.TextIO):
    # Sort by size change so biggest is at top. But preserve implicit
    # hierarchy in the ordering, by grouping items
    for region, change, size_change in (
        element
        for _region, grouper in groupby(diff, key=lambda r_c_v: r_c_v[0].type)
        for element in sorted(grouper, key=lambda r_c_v: r_c_v[2], reverse=True)
    ):
        size = utils.binary_prefix(size_change)
        desc = f"{region.type} {shorten_with_details(region)}"
        if region.type == "area":
            md_out.write(f"### {change.capitalize()} {size}B due to {desc}\n\n")
        elif region.type == "output":
            md_out.write(f"\n{change.capitalize()} {size}B due to {desc}\n\n")
        else: # region.type == "input"
            md_out.write(f"- {change.capitalize()} {size}B due to {desc}\n")

def svg(mapfile: MapFile, diff: Diff, svg_out: t.TextIO):
    random = Random(1234)

    grew = set()
    added = set()

    for region, change, _value in diff:
        if change == Change.Add:
            added.add(region)
        elif change == Change.Increase:
            grew.add(region)

    def palette(entry: Area | Section) -> str:
        r = random.randint(0, 0x20)
        g = random.randint(0, 0x20)
        b = random.randint(0, 0x80)
        if entry in grew:
            g += 0x7F
        elif entry in added:
            g += 0xBF
        return f"#{r:02X}{g:02X}{b:02X}"

    write_svg(mapfile, palette, svg_out)
