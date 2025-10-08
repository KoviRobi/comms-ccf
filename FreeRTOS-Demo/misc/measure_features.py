#!/usr/bin/env python

import os
import sys
from pathlib import Path
from shlex import split
from subprocess import run

scriptdir = Path(__file__).parent
sourcedir = scriptdir.parent
os.chdir(sourcedir)

# TODO: Set up properly, this is fine for now.
sys.path.append("../python/binutils_mapfile")

from parser import MapFile
from diff import compare, md, svg

build = "minsizerel"
run(split(f"cmake --preset {build}"), check=True)
run(split(f"cmake --build --preset {build} --target FreeRTOS-Demo-measure"), check=True)

prefix = "FreeRTOS-Demo-"
suffix = ".elf.map"
dir = sourcedir / "build" / build
glob = dir.glob(prefix + "*" + suffix)


def globkey(p: Path) -> int:
    return int(p.name.removeprefix(prefix).removesuffix(suffix))


md_file = scriptdir / "compare.md"

with md_file.open("wt") as md_out:
    prev = None
    p_file = None
    m = None
    files = list(sorted(glob, key=globkey))

    md_out.write(f"# Comparing sizes for build {build}\n\n")
    if files:
        md_out.write(f"Files looked at (in {dir.relative_to(sourcedir)}):\n")
        for file in files:
            md_out.write(f"- {file.relative_to(dir)}\n")
    else:
        md_out.write("No files to compare!\n")

    for file in files:
        file = file.relative_to(sourcedir)  # shorten name
        n = globkey(file)
        with file.open("rt") as fp:
            mapfile = MapFile.parse(fp)
        if prev is not None and p_file is not None and m is not None:
            diff = compare(prev, mapfile)
            md_out.write(f"\n## Comparing {p_file.name} to {file.name}")
            md(diff, md_out)
            with (scriptdir / f"compare-{m}-{n}.svg").open("wt") as svg_out:
                svg(mapfile, diff, svg_out)
        prev, p_file, m = mapfile, file, n
