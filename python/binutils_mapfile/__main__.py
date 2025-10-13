"""
Tool for analysing a mapfile, or comparing two mapfiles.
"""

import re
import sys
from argparse import ArgumentParser
from glob import glob
from parser import MapFile
from pathlib import Path

from diff import compare
from diff import md as diff_md
from diff import svg as diff_svg
from svg import svg
from yaml import diffable_format


def extract_number(s: str) -> tuple[tuple[int, ...], str]:
    """
    Try to do human-numeric sort: Extract digits first, then the whole
    string as a backup.
    """
    return (tuple(int(d) for d in re.findall(r"\d+", s)), s)


def main():
    """
    Parse the given mapfile and format it in a way which is amenable
    to diff
    """
    parser = ArgumentParser()
    parser.add_argument("mapfile", nargs="+", type=glob)
    parser.add_argument("--yaml", type=Path, help="Output a YAML suitable for diff")
    parser.add_argument("--svg", type=Path, help="Output an SVG treemap")
    args = parser.parse_args()
    mapfiles = [
        Path(mapfile)
        for glob in args.mapfile
        for mapfile in sorted(glob, key=extract_number)
    ]

    previous = None
    for n, mapfile in enumerate(mapfiles):
        with mapfile.open("rt") as fp:
            mapfile = MapFile.parse(fp)
            diff = None
            if previous is not None:
                diff = compare(previous, mapfile)
            if not args.yaml and not args.svg:
                diffable_format(mapfile, sys.stdout)
            if args.yaml:
                yaml_file: Path = args.yaml
                yaml_file.parent.mkdir(exist_ok=True, parents=True)
                if len(mapfiles) > 1:
                    yaml_file = args.yaml.with_suffix(f".{n}{args.yaml.suffix}")
                with yaml_file.open("wt") as yaml_out:
                    diffable_format(mapfile, yaml_out)
                if diff is not None:
                    yaml_file = args.yaml.with_suffix(f".{n-1}-{n}{args.yaml.suffix}")
                    with yaml_file.open("wt") as yaml_out:
                        yaml_out.write(f"## Comparing {mapfiles[n-1]} to {mapfiles[n]}\n\n")
                        diff_md(diff, yaml_out)
            if args.svg:
                svg_file: Path = args.svg
                svg_file.parent.mkdir(exist_ok=True, parents=True)
                if len(mapfiles) > 1:
                    svg_file = args.svg.with_suffix(f".{n}{args.svg.suffix}")
                with svg_file.open("wt") as svg_out:
                    svg(mapfile, svg_out)
                    if diff is not None:
                        svg_file = args.svg.with_suffix(f".{n-1}-{n}{args.svg.suffix}")
                        with svg_file.open("wt") as svg_out:
                            diff_svg(mapfile, diff, svg_out)

            previous = mapfile


if __name__ == "__main__":
    main()
