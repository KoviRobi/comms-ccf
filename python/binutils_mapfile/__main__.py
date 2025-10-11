"""
Tool for analysing a mapfile, or comparing two mapfiles.
"""

import sys
from argparse import ArgumentParser
from parser import MapFile
from pathlib import Path

from diff import compare
from svg import svg
from yaml import diffable_format


def main():
    """
    Parse the given mapfile and format it in a way which is amenable
    to diff
    """
    parser = ArgumentParser()
    parser.add_argument("mapfile", type=Path)
    parser.add_argument("--yaml", type=Path, help="Output a YAML suitable for diff")
    parser.add_argument("--svg", type=Path, help="Output an SVG treemap")
    args = parser.parse_args()
    with args.mapfile.open("rt") as fp:
        global mapfile  # for `python -i <this file> <mapfile>`
        mapfile = MapFile.parse(fp)
        if not args.yaml and not args.svg:
            diffable_format(mapfile, sys.stdout)
        if args.yaml:
            with args.yaml.open("wt") as file:
                diffable_format(mapfile, file)
        if args.svg:
            with args.svg.open("wt") as file:
                svg(mapfile, palette, file=file)


if __name__ == "__main__":
    main()
