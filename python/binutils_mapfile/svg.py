from __future__ import annotations

import sys
import typing as t
from parser import MapFile
from pathlib import Path

import utils
from squarify import normalize_sizes, squarify


def svg(
    mapfile: MapFile,
    palette,
    file: t.TextIO = sys.stdout,
    width: int = 297,
    height: int = 210,
    area_overlay: bool = True,
    output_overlay: bool = True,
    input_overlay: bool = True,
):
    def indent(*args):
        return utils.indent(*args, level=2, file=file, strip=4)

    indent('<?xml version="1.0" encoding="UTF-8"?>')
    indent(
        f'<svg width="{width}mm" height="{height}mm" viewBox="0 0 {width} {height}"',
        'preserveAspectRatio="none" version="1.1" xmlns="http://www.w3.org/2000/svg">',
    )
    # Crosshatch pattern
    indent("<defs>")
    indent(
        '    <pattern id="crosshatch" width="5" height="1" ',
        'patternTransform="rotate(45)" patternUnits="userSpaceOnUse"',
        'preserveAspectRatio="xMidYMid">',
    )
    indent('        <rect width="1" height="1" fill="#000000"/>')
    indent("    </pattern>")
    indent("</defs>")
    css = Path(__file__).parent / "svg.css"
    with css.open("rt") as fp:
        indent("<style>")
        while line := fp.readline():
            indent(line.rstrip())
        indent("</style>")
    indent(f'<rect width="{width}" height="{height}" fill="url(#crosshatch)"/>')
    # The areas aren't square maps, just one after another
    areas = list(mapfile.iter_area_sections())
    area_height = height / len(areas)
    area_x = 0
    for n, (area, output_sections) in enumerate(areas):
        area_name = area.name.strip('"')
        area_short = utils.ellipsise_templates(area_name)
        area_colour = palette(area)
        area_id = f"area-{area.name}-{area.origin}"
        sorted_outputs = list(
            sorted(output_sections, key=lambda s: s.size)
        )
        output_sizes = [section.size for section in sorted_outputs]
        area_used = sum(output_sizes)
        area_width = width * area_used / area.length
        area_y = n * area_height

        area_desc = area_short
        if area_short != area_name:
            area_desc += "\n" + area_name
        area_desc += f"\n0x{area.origin:08X}"
        area_desc += f" + 0x{area_used:X}"
        area_desc += f" ({utils.binary_prefix(area_used)},"
        area_desc += f" {100*area_used/area.length:.2f}% used)"

        output_normed = normalize_sizes(output_sizes, 1, 1)
        output_rects = list(zip(squarify(output_normed, 0, 0, 1, 1), sorted_outputs))
        indent(
            f'<g id="{utils.xmlescape(area_id)}_group" class="area group" transform="'
            f"translate({area_x} {area_y})",
            f'scale({area_width} {area_height})">',
        )
        indent(
            f'<view id="{utils.xmlescape(area_id)}" viewBox="'
            f'{area_x} {area_y} {area_width} {area_height}"/>',
        )
        for output_rect, output_section in output_rects:
            output_name = output_section.pretty_name()
            output_short = utils.ellipsise_templates(output_name)

            output_desc = output_short
            if output_short != output_name:
                output_desc += "\n" + output_name
            output_desc += f"\n0x{output_section.address:08X}"
            output_desc += f" + 0x{output_section.size:08X}"
            output_desc += (
                f" ({utils.binary_prefix(output_section.size)}B,"
                + f" {100*output_section.size/area_used:.2f}% used"
                + f"  {100*output_section.size/area.length:.2f}% total of {area_name})"
            )
            if output_section.fill > 0:
                output_desc += f"\nfill: {output_section.fill:08}"

            output_colour = palette(output_section)
            output_id = f"{area_id}-output-{output_name}-{output_section.address}"
            indent(
                f'<g id="{utils.xmlescape(output_id)}_group" class="output group" transform="'
                f'translate({output_rect["x"]} {output_rect["y"]})',
                f'scale({output_rect["dx"]} {output_rect["dy"]})">',
            )
            indent(
                f'<view id="{utils.xmlescape(output_id)}" viewBox="'
                f'{area_x + area_width * output_rect["x"]}',
                f'{area_y + area_height * output_rect["y"]}',
                f'{area_width * output_rect["dx"]}',
                f'{area_height * output_rect["dy"]}"/>',
            )
            sorted_inputs = list(
                sorted(output_section.inputs, key=lambda s: s.size, reverse=True)
            )
            input_sizes = [section.size for section in sorted_inputs]
            input_normed = normalize_sizes(input_sizes, 1, 1)
            input_rects = zip(squarify(input_normed, 0, 0, 1, 1), sorted_inputs)
            for input_rect, input_section in input_rects:
                input_name = input_section.pretty_name(output_section)
                input_short = utils.ellipsise_templates(input_name)

                input_desc = input_short
                if input_short != input_name:
                    input_desc += "\n" + input_name
                path = Path(input_section.object).resolve()
                if input_section.object != path.name:
                    input_desc += "\n"
                    if input_name != path.name:
                        input_desc += path.name + " "
                    input_desc += "from " + str(path.parent)
                else:
                    input_desc += "\n" + input_section.object
                input_desc += f"\n0x{input_section.address:08X}"
                input_desc += f" + 0x{input_section.size:08X}"
                input_desc += (
                    f" ({utils.binary_prefix(input_section.size)}B,"
                    + f"{100*input_section.size/output_section.size:.2f}% of {output_name},"
                    + f" {100*input_section.size/area_used:.2f}% used,"
                    + f" {100*input_section.size/area.length:.2f}% total of {area_name})"
                )
                if input_section.fill > 0:
                    input_desc += f"\nfill: {input_section.fill:08}"
                for sym in input_section.symbols:
                    input_desc += f"\n{sym.name} 0x{sym.address:X} {sym.wildcard}"

                input_colour = palette(input_section)
                input_id = f"{output_id}-input-{input_name}-{input_section.address}"
                indent(
                    f'<g id="{utils.xmlescape(input_id)}_group" class="input group" transform="'
                    f'translate({input_rect["x"]} {input_rect["y"]})',
                    f'scale({input_rect["dx"]} {input_rect["dy"]})">',
                )
                indent(
                    f'<view id="{utils.xmlescape(input_id)}" viewBox="'
                    f'{area_x + area_width * output_rect["x"] + area_width * output_rect["dx"] * input_rect["x"]}',
                    f'{area_y + area_height * output_rect["y"] + area_height * output_rect["dy"] * input_rect["y"]}',
                    f'{area_width * output_rect["dx"] * input_rect["dx"]}',
                    f'{area_height * output_rect["dy"] * input_rect["dy"]}"/>',
                )
                if input_overlay:
                    indent(
                        f'<a href="#{utils.xmlescape(input_id)}"><rect',
                        f'id="{utils.xmlescape(input_id)}_rect"',
                        f'class="input rect" fill="{input_colour}"',
                        'x="0" y="0" width="1" height="1">'
                        f"<title>{utils.xmlescape(input_desc)}</title></rect>",
                    )
                    indent(
                        '<text y="0.6" font-size="0.4" stroke-width="0.002"',
                        'fill="#000000" stroke="#FFFFFF"',
                        'textLength="1" lengthAdjust="spacingAndGlyphs">'
                        f"<title>{utils.xmlescape(input_desc)}</title><tspan>"
                        f"{utils.xmlescape(input_short)}</tspan></text></a>",
                    )
                indent("</g>")
            if output_overlay:
                indent(
                    f'<a class="obscures" href="#{utils.xmlescape(output_id)}">'
                    f'<rect id="{utils.xmlescape(output_id)}_rect"',
                    f'class="output rect" fill="{output_colour}"',
                    'x="0" y="0" width="1" height="1">',
                    f"<title>{utils.xmlescape(output_desc)}</title></rect>",
                )
                indent(
                    '<text y="0.6" font-size="0.4" stroke-width="0.002"',
                    'fill="#000000" stroke="#FFFFFF"',
                    'textLength="1" lengthAdjust="spacingAndGlyphs">'
                    f"<title>{utils.xmlescape(output_desc)}</title><tspan>",
                    f"{utils.xmlescape(output_short)}</tspan></text></a>",
                )
            indent("</g>")
        if area_overlay:
            indent(
                f'<a class="obscures" href="#{utils.xmlescape(area_id)}">'
                f'<rect id="{utils.xmlescape(area_id)}_rect"',
                f'class="area rect" fill="{area_colour}"',
                f'x="0" y="0" width="1" height="1">',
                f"<title>{utils.xmlescape(area_desc)}</title></rect>",
            )
            indent(
                f'<text y="0.6" font-size="0.4" stroke-width="0.002"',
                'fill="#000000" stroke="#FFFFFF">'
                f"<title>{utils.xmlescape(area_desc)}</title><tspan>"
                f"{utils.xmlescape(area_short)}</tspan></text></a>",
            )
        indent("</g>")
    js = Path(__file__).parent / "svg.js"
    with js.open("rt") as fp:
        indent("<script>")
        indent("// <![CDATA[")
        while line := fp.readline():
            indent(line.rstrip())
        indent("// ]]>")
        indent("</script>")
    indent("</svg>")
