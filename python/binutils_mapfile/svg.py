"""
Plot a mapfile as a tree map using squarify
"""

from __future__ import annotations

import sys
import typing as t
from dataclasses import asdict, dataclass
from random import Random

from jinja2 import Environment, PackageLoader, select_autoescape
from squarify import normalize_sizes, squarify

import binutils_mapfile.utils as utils
from binutils_mapfile.parser import Area, InputSection, MapFile, OutputSection

Region = Area | OutputSection | InputSection

random = Random(1234)


def random_palette(_) -> str:
    return f"#{random.randint(0, ((1 << 24) - 1)):06X}"


env = Environment(
    loader=PackageLoader("binutils_mapfile"),
    autoescape=select_autoescape(default=True),
)


@dataclass
class SvgRect:
    name: str
    short: str
    description: str
    colour: str
    x: float
    y: float
    w: float
    h: float


@dataclass
class SvgInputSection(SvgRect):
    pass


@dataclass
class SvgOutputSection(SvgRect):
    input_sections: tuple[SvgInputSection, ...]


@dataclass
class SvgArea(SvgRect):
    output_sections: tuple[SvgOutputSection, ...]


@dataclass
class SvgOutput:
    width: int
    height: int
    js: str
    css: str
    areas: tuple[SvgArea, ...]
    area_overlay: bool
    input_overlay: bool
    output_overlay: bool


def svg(
    mapfile: MapFile,
    file: t.TextIO = sys.stdout,
    palette: t.Callable[[Region], str] = random_palette,
    width: int = 297,
    height: int = 210,
    area_overlay: bool = True,
    output_overlay: bool = True,
    input_overlay: bool = True,
    extra_tooltip: None | t.Callable[[Region], None | str] = None,
):
    # The areas aren't square maps, just one after another
    areas = list(mapfile.iter_area_sections())
    area_height = height / len(areas)
    area_x = 0
    svg_areas: list[SvgArea] = []
    for n, (area, output_sections) in enumerate(areas):
        area_name = area.name.strip('"')
        area_short = utils.ellipsise_templates(area_name)
        area_colour = palette(area)
        sorted_outputs = list(sorted(output_sections, key=len))
        output_sizes = [len(section) for section in sorted_outputs if len(section) > 0]
        area_used = sum(output_sizes)
        area_width = width * area_used / len(area)
        area_y = n * area_height

        area_desc = area_short
        if area_short != area_name:
            area_desc += "\n" + area_name
        area_desc += f"\n0x{area.origin:08X}"
        area_desc += f" + 0x{area_used:X}"
        area_desc += f" ({utils.binary_prefix(area_used)},"
        area_desc += f" {100*area_used/len(area):.2f}% used)"

        extra_area_desc = extra_tooltip
        if isinstance(extra_tooltip, t.Callable):
            extra_area_desc = extra_tooltip(area)
        if isinstance(extra_area_desc, str):
            area_desc += "\n" + extra_area_desc

        output_normed = normalize_sizes(output_sizes, 1, 1)
        output_rects = list(zip(squarify(output_normed, 0, 0, 1, 1), sorted_outputs))
        svg_output_sections: list[SvgOutputSection] = []
        for output_rect, output_section in output_rects:
            output_name = output_section.pretty_name()
            output_short = utils.ellipsise_templates(output_name)

            output_desc = output_short
            if output_short != output_name:
                output_desc += "\n" + output_name
            output_desc += f"\n0x{output_section.address:08X}"
            output_desc += f" + 0x{len(output_section):08X}"
            output_desc += (
                f" ({utils.binary_prefix(len(output_section))}B,"
                + f" {100*len(output_section)/area_used:.2f}% used"
                + f"  {100*len(output_section)/len(area):.2f}% total of {area_name})"
            )
            if output_section.fill > 0:
                output_desc += f"\nfill: {output_section.fill:08}"

            extra_output_desc = extra_tooltip
            if isinstance(extra_tooltip, t.Callable):
                extra_output_desc = extra_tooltip(output_section)
            if isinstance(extra_output_desc, str):
                output_desc += "\n" + extra_output_desc

            output_colour = palette(output_section)
            sorted_inputs = list(sorted(output_section.inputs, key=len, reverse=True))
            input_sizes = [
                len(section) for section in sorted_inputs if len(section) > 0
            ]
            input_normed = normalize_sizes(input_sizes, 1, 1)
            input_rects = zip(squarify(input_normed, 0, 0, 1, 1), sorted_inputs)
            svg_input_sections: list[SvgInputSection] = []
            for input_rect, input_section in input_rects:
                input_name = input_section.pretty_name(output_section.pretty_name())
                input_short = utils.ellipsise_templates(input_name)

                input_desc = input_short
                if input_short != input_name:
                    input_desc += "\n" + input_name
                input_desc += "\n"
                if len(input_section.object.parts) > 1:
                    if input_name != input_section.object.name:
                        input_desc += input_section.object.name + " "
                    input_desc += "from " + str(input_section.object.parent)
                else:
                    input_desc += str(input_section.object)
                input_desc += f"\n0x{input_section.address:08X}"
                input_desc += f" + 0x{len(input_section):08X}"
                input_desc += (
                    f" ({utils.binary_prefix(len(input_section))}B,"
                    + f"{100*len(input_section)/len(output_section):.2f}% of {output_name},"
                    + f" {100*len(input_section)/area_used:.2f}% used,"
                    + f" {100*len(input_section)/len(area):.2f}% total of {area_name})"
                )
                if input_section.fill > 0:
                    input_desc += f"\nfill: {input_section.fill:08}"
                for sym in input_section.symbols:
                    input_desc += f"\n{sym.name} 0x{sym.address:X} {sym.wildcard}"

                extra_input_desc = extra_tooltip
                if isinstance(extra_tooltip, t.Callable):
                    extra_input_desc = extra_tooltip(input_section)
                if isinstance(extra_input_desc, str):
                    input_desc += "\n" + extra_input_desc
                input_colour = palette(input_section)
                svg_input_sections.append(
                    SvgInputSection(
                        name=input_name,
                        short=input_short,
                        description=input_desc,
                        colour=input_colour,
                        x=input_rect["x"],
                        y=input_rect["y"],
                        w=input_rect["dx"],
                        h=input_rect["dy"],
                    )
                )
            svg_output_sections.append(
                SvgOutputSection(
                    name=output_name,
                    short=output_short,
                    description=output_desc,
                    colour=output_colour,
                    x=output_rect["x"],
                    y=output_rect["y"],
                    w=output_rect["dx"],
                    h=output_rect["dy"],
                    input_sections=tuple(svg_input_sections),
                )
            )
        svg_areas.append(
            SvgArea(
                name=area_name,
                short=area_short,
                description=area_desc,
                colour=area_colour,
                x=area_x,
                y=area_y,
                w=area_width,
                h=area_height,
                output_sections=tuple(svg_output_sections),
            )
        )
    stream = env.get_template("squaremap.svg.jinja").stream(
        asdict(
            SvgOutput(
                width=width,
                height=height,
                css=env.get_template("squaremap.css.jinja").render(),
                js=env.get_template("squaremap.js.jinja").render(),
                areas=tuple(svg_areas),
                area_overlay=area_overlay,
                input_overlay=input_overlay,
                output_overlay=output_overlay,
            )
        )
    )
    # The `dump` is annotated as a `BinartIO` but if I don't pass in
    # the encoding it doesn't call `encode` so it expects a StringIO
    fp = t.cast(t.BinaryIO, file)
    stream.dump(fp=fp)
    # For some reason Jinja doesn't write the final new-line -- perhaps
    # because `dump` expects a binary IO not a text IO
    file.write("\n")
