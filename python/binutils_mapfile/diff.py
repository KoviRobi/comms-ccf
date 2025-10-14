"""
Compare two different mapfiles, outputting a markdown or an SVG summary
"""

import typing as t
from collections import defaultdict
from dataclasses import dataclass, field
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


Region = Area | OutputSection | InputSection


@dataclass(frozen=True)
class Hunk:
    """One item of a diff, name comes from `diff` tool"""
    region: Region
    change: Change
    value: int


Diff = list[Hunk]


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
                diff.append(Hunk(new_out_sec, Change.Add, len(new_out_sec)))
                continue
            old_out_sec = old_out_secs.pop(new_out_sec.name)
            size_increase = len(new_out_sec) - len(old_out_sec)
            if size_increase == 0:
                continue
            elif size_increase < 0:
                diff.append(Hunk(new_out_sec, Change.Decrease, -size_increase))
            else:  # size_increase > 0
                diff.append(Hunk(new_out_sec, Change.Increase, size_increase))

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
                    diff.append(Hunk(new_in_sec, Change.Add, len(new_in_sec)))
                    continue
                old_in_sec = old_in_secs.pop(key)
                size_increase = len(new_in_sec) - len(old_in_sec)
                if len(old_in_sec) == len(new_in_sec):
                    continue
                elif size_increase < 0:
                    diff.append(Hunk(new_in_sec, Change.Decrease, -size_increase))
                else:  # size_increase > 0
                    diff.append(Hunk(new_in_sec, Change.Increase, size_increase))
            for removed in old_in_secs.values():
                diff.append(Hunk(removed, Change.Remove, len(removed)))
        for removed in old_out_secs.values():
            diff.append(Hunk(removed, Change.Remove, len(removed)))
        size_increase = len(new_area) - len(old_area)
        if size_increase < 0:
            diff.append(Hunk(new_area, Change.Decrease, size_increase))
        elif size_increase > 0:
            diff.append(Hunk(new_area, Change.Increase, size_increase))

    # If size changed (or even if it didn't) because template parameters
    # changed, we will see a remove and an addition. Here we try to
    # detect them.
    @dataclass
    class Renames:
        # For add/remove the size is just the same as len(region) anyway
        add: list[Region] = field(default_factory=list)
        remove: list[Region] = field(default_factory=list)

    # First we group potential matches by their short name (multiple
    # templates may have the same short name).
    renames: defaultdict[str, Renames] = defaultdict(Renames)
    for hunk in diff:
        if hunk.change not in [Change.Add, Change.Remove]:
            continue
        name = hunk.region.pretty_name()
        short_name = utils.ellipsise_templates(name)
        if name == short_name:
            # No templates, can be ignored
            continue
        rename = renames[short_name]
        if hunk.change == Change.Add:
            rename.add.append(hunk.region)
        elif hunk.change == Change.Remove:
            rename.remove.append(hunk.region)

    # Then we remove one half of a match (the removal, as that is
    # the old region/address), and change the addition to a size
    # increase/decrease.
    to_remove: set[Region] = set()
    # Region to size increase/decrease
    to_change: dict[Region, int] = {}
    for _short, rename in renames.items():
        for add, remove in zip(
            sorted(rename.add, key=len), sorted(rename.remove, key=len)
        ):
            size_increase = len(add) - len(remove)
            # The region added is always the new region
            to_change[add] = size_increase
            to_remove.add(remove)

    def update(hunk: Hunk) -> Hunk | None:
        size_increase = to_change.get(hunk.region, None)
        if size_increase is None:
            return hunk
        if size_increase == 0:
            return None
        if size_increase < 0:
            return Hunk(hunk.region, Change.Decrease, -size_increase)
        else:
            return Hunk(hunk.region, Change.Increase, size_increase)

    diff = [
        hunk
        for hunk in map(update, diff)
        if hunk is not None
        if hunk.region not in to_remove
    ]

    return diff


def md(diff: Diff, md_out: t.TextIO):
    """
    Outputs a markdown/yaml file.
    """
    # Sort by size change so biggest is at top. But preserve implicit
    # hierarchy in the ordering, by grouping items
    for hunk in (
        element
        for _region, grouper in groupby(diff, key=lambda hunk: hunk.region.type)
        for element in sorted(grouper, key=lambda hunk: hunk.value, reverse=True)
    ):
        size = utils.binary_prefix(hunk.value)
        desc = f"{hunk.region.type} {shorten_with_details(hunk.region)}"
        if hunk.region.type == "area":
            md_out.write(f"### {hunk.change.capitalize()} {size}B due to {desc}\n\n")
        elif hunk.region.type == "output":
            md_out.write(f"\n- {hunk.change.capitalize()} {size}B due to {desc}:\n\n")
        elif hunk.region.type == "input":
            md_out.write(f"  - {hunk.change.capitalize()} {size}B due to {desc}\n")
        else:
            assert f"Unknown region type {hunk.region.type}"


def svg(mapfile: MapFile, diff: Diff, svg_out: t.TextIO):
    random = Random(1234)

    added = set()
    grew = set()
    shrunk = set()

    for hunk in diff:
        if hunk.change == Change.Add:
            added.add(hunk.region)
        elif hunk.change == Change.Increase:
            grew.add(hunk.region)
        elif hunk.change == Change.Decrease:
            shrunk.add(hunk.region)

    objects: dict[Path, int] = {}
    def palette(entry: Area | Section) -> str:
        color = random.randint(0, 0xFFFFFF)
        if isinstance(entry, Section):
            color = color & 0x1F1FFF
            objects[entry.object] = objects.get(entry.object, color)
            if entry in added:
                color = color | 0x00E000
            elif entry in grew:
                color = color | 0x00A000
            elif entry in shrunk:
                color = color | 0xA00000
        return f"#{color:06X}"

    write_svg(mapfile, svg_out, palette)
