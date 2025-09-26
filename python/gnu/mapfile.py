from __future__ import annotations

import re
import typing as t
from argparse import ArgumentParser
from dataclasses import dataclass, field
from pathlib import Path
from pprint import pprint
from intervaltree.intervaltree import Interval, IntervalTree

import squarify

BITS = 32
MEMDIGITS = 2 * BITS // 8  # 32 bits in base16
HEXDIGITS = 2 + MEMDIGITS  # "0x" + memdigits

# See binutils/ld/ldmain.c add_archive_element
REFERENCE_ALIGN = 30

# TODO: I think theoretically ELF section names could be any C string,
# but this will do for the common case.
SECTION_NAME_RE = re.compile(r"^[0-9a-zA-Z._]+($|(?= ))")
# See binutils/ld/ldlang.h SECTION_NAME_MAP_LENGTH
SECTION_NAME_LEN = 16
# 16 actual spaces in binutils/ld/ldlang.c print_one_symbol
SYMBOL_NAME_SEP = 16
# Two spaces plus buffer up to 32 characters binutils/ld/ldlang.c
# print_assignment
# 0x01234567(10) | # [0x01234567](12) | [unresolved](12) | *undef*(7)
ASSIGNMENT_NAME_SEP = 2 + 32 - 12


class ArchiveMembers:
    """
    See binutils/ld/ldmain.c add_archive_element
    Format is either (no space before/after [/])

        object [pad to 30 columns] object (symbol)

    or if archive(object) is longer than 30 columns

        object \n [pad to 30 columns] object (symbol)

    where object is either archive(member) or just object if it is an
    object file
    """

    @dataclass
    class Reference:
        _REFERENCE_RE = re.compile(r"^(?P<object>.*) \((?P<symbol>.*)\)$")
        # Object or archive(object) file referencing the symbol
        object: str
        # Symbol being referenced
        symbol: str

        @classmethod
        def parse(cls, line: str) -> t.Optional[ArchiveMembers.Reference]:
            if m := cls._REFERENCE_RE.match(line):
                return cls(**m.groupdict())

    def __init__(self) -> None:
        # A dict showing why the key is being pulled in: the reference
        # contains the object/archive(object) file and the symbol used
        # by said object
        self.archives: dict[str, self.Reference] = {}

    def feed(self, line: str, lines: t.Iterator[str]):
        if line is None or line == "":
            return
        if (
            len(line) >= REFERENCE_ALIGN - 1
            and line[0] != " "
            and line[REFERENCE_ALIGN - 2] == " "
            and line[REFERENCE_ALIGN - 1] == " "
        ):
            definition = line[:REFERENCE_ALIGN].rstrip()
            reference = line[REFERENCE_ALIGN:]
        else:
            definition = line
            try:
                reference = next(lines)[REFERENCE_ALIGN:]
            except StopIteration:
                return
        self.archives[definition] = self.Reference.parse(reference)


@dataclass(frozen=True)
class Symbol:
    name: str
    address: int
    wildcard: str


@dataclass
class Section:
    _REFERENCE_RE = re.compile(r"^(?P<object>.*) \((?P<symbol>.*)\)$")
    name: str
    address: int
    size: int
    object: str

    @classmethod
    def parse(cls, name: str, line: str, lines: t.Iterator[str]) -> Section:
        if len(name) >= SECTION_NAME_LEN - 1:
            line = next(lines)
        line = line[SECTION_NAME_LEN:]
        addr = int(line[:HEXDIGITS], 0)
        size = int(line[HEXDIGITS + 1 : 2 * HEXDIGITS + 1], 0)
        object = line[2 * HEXDIGITS + 2 :]
        return cls(name.strip(), addr, size, object)


class DiscardedSection(Section):
    pass


@dataclass
class InputSection(Section):
    symbols: list[Symbol] = field(default_factory=list)
    padding: int = 0


class OutputSection(Section):
    pass


class Discards:
    """
    See binutils/ld/ldlang.c print_input_section (with is_discarded = true)
    """

    _REFERENCE_ALIGN = 30

    def __init__(self) -> None:
        self.discards = {}

    def feed(self, line: str, lines: t.Iterator[str]):
        if line is None or line == "":
            return
        elif line[0] == " " and (m := SECTION_NAME_RE.match(line[1:])):
            discard = DiscardedSection.parse(" " + m[0], line, lines)
            self.discards[discard.name] = discard


@dataclass(frozen=True)
class Area:
    name: str
    origin: int
    length: int
    attributes: frozenset[str]

    @classmethod
    def parse(cls, name: str, origin: str, length: str, attributes: str = ""):
        return cls(
            name=name,
            origin=int(origin, 0),
            length=int(length, 0),
            attributes=frozenset(attributes),
        )


class MemoryConfiguration:
    """
    See binutils/ld/ldlang.c lang_map. Format is something like:

            Name             Origin             Length             Attributes
            FLASH            0x00000000         0x00040000         xr
            SRAM             0x20000000         0x00010000         xrw
            *default*        0x00000000         0xffffffff
    """

    def __init__(self) -> None:
        self.header: None | list[str] = None
        self.areas = []

    def feed(self, line: str, _lines: t.Iterator[str]):
        if not line:
            return
        if self.header is None:
            self.header = line.lower().split()
        else:
            kwargs = dict(zip(self.header, line.split()))
            try:
                area = Area.parse(**kwargs)
            except TypeError as e:
                args = ", ".join(map(": ".join, kwargs.items()))
                raise RuntimeError(f"Failed to parse Area with {args}", e)
            self.areas.append(area)


class MemoryMap:
    def __init__(self) -> None:
        self.sections: list[Section] = []

    def feed(self, line: str, lines: t.Iterator[str]):
        if line is None or line == "":
            return
        elif any(line.startswith(op) for op in ["START GROUP", "LOAD", "END GROUP"]):
            return
        elif line.startswith("/DISCARD/"):
            return
        elif m := SECTION_NAME_RE.match(line):
            self.new_output_section(m[0], line, lines)
        elif line[0] == " " and (m := SECTION_NAME_RE.match(line[1:])):
            self.new_input_section(" " + m[0], line, lines)
        elif (
            len(line) > SECTION_NAME_LEN + HEXDIGITS
            and line[: SECTION_NAME_LEN + HEXDIGITS].isspace()
        ):
            self.relaxed_symbol(line[SECTION_NAME_LEN + HEXDIGITS :])
        elif len(line) > SECTION_NAME_LEN and line[:SECTION_NAME_LEN].isspace():
            line = line[SECTION_NAME_LEN:]
            sep = line[HEXDIGITS:]
            if sep[:ASSIGNMENT_NAME_SEP].isspace():
                self.add_assignment(line)
            elif sep[:SYMBOL_NAME_SEP].isspace():
                self.add_symbol(line)
        elif "(" in line:
            self.wildcard_section(line)
        elif line[:8] == " *fill* ":
            self.add_fill(line)
        else:
            print("UNKNOWN       ", line)

    def new_output_section(self, name: str, line: str, lines: t.Iterator[str]):
        "See binutils/ld/ldlang.c print_output_section_statement"
        self.sections.append(OutputSection.parse(name, line, lines))

    def wildcard_section(self, line: str):
        "See binutils/ld/ldlang.c print_wild_statement"
        self._wildcard = line.strip()

    def new_input_section(self, section: str, line: str, lines: t.Iterator[str]):
        "See binutils/ld/ldlang.c print_intput_statement"
        self.sections.append(InputSection.parse(section, line, lines))

    def relaxed_symbol(self, line: str):
        "See binutils/ld/ldlang.c print_input_section"
        _size = int(line[:HEXDIGITS], 0)
        if "(size before relaxing)" not in line:
            print("Was expecting size before relaxing")

    def add_assignment(self, line: str):
        "See binutils/ld/ldlang.c print_one_symbol"
        # TODO: [addr], [unresolved], or *undef*
        return
        addr = int(line[:HEXDIGITS], 0)
        # +2 for [0x1234567] or [unresolved]
        assignment = line[HEXDIGITS + 2 + ASSIGNMENT_NAME_SEP :]
        print("Assignment    ", f"0x{addr:08X}", f"'{assignment}'")

    def add_symbol(self, line: str):
        "See binutils/ld/ldlang.c print_one_symbol"
        addr = int(line[:HEXDIGITS], 0)
        name = line[HEXDIGITS + SYMBOL_NAME_SEP :]
        assert self.sections != []
        input_section = self.sections[-1]
        assert isinstance(input_section, InputSection)
        input_section.symbols.append(Symbol(name, addr, self._wildcard))

    def add_fill(self, line: str):
        "See binutils/ld/ldlang.c print_padding_statement"
        line = line[SECTION_NAME_LEN:]
        _addr = int(line[:HEXDIGITS], 0)
        size = int(line[HEXDIGITS + 1 : 2 * HEXDIGITS + 1], 0)
        assert self.sections != []
        input_section = self.sections[-1]
        assert isinstance(input_section, InputSection)
        input_section.padding += size


@dataclass
class MapFile:
    archive_members: dict[str, ArchiveMembers.Reference] = field(default_factory=dict)
    discards: dict[str, DiscardedSection] = field(default_factory=dict)
    memory_configuration: list[Area] = field(default_factory=list)
    memory_map: list[Section] = field(default_factory=list)

    @classmethod
    def parse(cls, file_like: t.TextIO) -> MapFile:
        self = cls()
        archive_members = ArchiveMembers()
        discards = Discards()
        memory_configuration = MemoryConfiguration()
        memory_map = MemoryMap()
        sections: dict[
            str, ArchiveMembers | Discards | MemoryConfiguration | MemoryMap
        ] = {
            "Archive member included to satisfy reference by file (symbol)": archive_members,
            "Discarded input sections": discards,
            "Memory Configuration": memory_configuration,
            "Linker script and memory map": memory_map,
        }
        parser = None

        lines = (line[:-1] for line in iter(file_like.readline, ""))
        for line in lines:
            if line in sections:
                parser = sections[line]
                continue
            if line.startswith("OUTPUT("):
                # Only orphan sections after here
                # TODO: Verify
                break
            if parser is not None:
                parser.feed(line, lines)
        return MapFile(
            archive_members=archive_members.archives,
            discards=discards.discards,
            memory_configuration=memory_configuration.areas,
            memory_map=memory_map.sections,
        )


def main():
    """
    Parse the given mapfile and format it in a way which is amenable
    to diff
    """
    parser = ArgumentParser()
    parser.add_argument("mapfile", type=Path)
    args = parser.parse_args()
    with args.mapfile.open("rt") as fp:
        global mapfile  # for `python -i <this file> <mapfile>`
        mapfile = MapFile.parse(fp)
        referrers = {
            reference.symbol: (reference.object, definition)
            for definition, reference in mapfile.archive_members.items()
        }
        areas = IntervalTree(
            Interval(area.origin, area.origin + area.length, (area, []))
            for area in mapfile.memory_configuration
            if area.attributes != set()
        )
        for section in mapfile.memory_map:
            for start, end, (area, area_sections) in areas[
                section.address : section.address + section.size
            ]:
                area_sections.append(section)
        for start, end, (area, sections) in areas:
            area = t.cast(Area, area)
            sections = t.cast(list[Section], sections)
            print(area.name)
            for section in sorted(sections, key=lambda sec: sec.size):
                if isinstance(section, InputSection):
                    print(
                        "-",
                        f"{type(section).__name__.lower()[:-7]:6}",
                        f"{section.name:30.30}",
                        # Don't print address to make diffs easier
                        # f"0x{section.address:08X}",
                        f"{section.size:08X}",
                        f"{100.0 * section.size / (end - start):7.3f}",
                    )
                    for sym in section.symbols:
                        name = sym.name
                        innermost_angle = re.compile(r"(.*)<[^>]*>(.*)")
                        while m := innermost_angle.match(name):
                            name = m[1] + "⟨...⟩" + m[2]
                        print(" ", "-", name)
                        if path := referrers.get(sym.name):
                            refr = Path(path[0]).name
                            if len(refr) > 30:
                                refr = f"{refr[:10]}...{refr[-27:]}"
                            defn = Path(path[1]).name
                            if len(defn) > 30:
                                defn = f"{defn[:10]}...{defn[-27:]}"
                            print(" ", " ", "-", refr, defn)


if __name__ == "__main__":
    main()


if False:
    input_sections = {
        sec.size: sec
        for sec in mapfile.memory_map
        if isinstance(sec, InputSection)
        if sec.size > 0
    }
    import matplotlib.pyplot as plt

    ax = squarify.plot(
        input_sections,
        label=[
            "\n".join(sym.name for sym in sec.symbols)
            for sec in input_sections.values()
        ],
    )
    plt.show()

    rects = list(
        zip(
            squarify.squarify(input_sections.keys(), 0, 0, 100, 100),
            input_sections.values(),
        )
    )
