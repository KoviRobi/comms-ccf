#!/usr/bin/env python3

"""
Parses linker mapfiles, and if invoked as a program, formats them into
a format more amenable to diff (e.g. by not printing addresses which
are subject to change).
"""

from __future__ import annotations

import re
import typing as t
from dataclasses import dataclass, field, replace
from pathlib import Path

from cxxfilt import InvalidName, demangle
from intervaltree.intervaltree import Interval, IntervalTree
from utils import IterWithPutBack

HEX_RE = re.compile(r"\s*(0x[0-9a-fA-F]+)\b")
RELAXED_RE = re.compile(r"\s*(0x[0-9a-fA-F]+)\s+\(size before relaxing\)")

# See binutils/ld/ldmain.c add_archive_element
REFERENCE_ALIGN = 30

# TODO: I think theoretically ELF section names could be any C string,
# but this will do for the common case.
SECTION_NAME_RE = re.compile(r'([0-9a-zA-Z._"/]+)\b')
# See binutils/ld/ldlang.h SECTION_NAME_MAP_LENGTH
SECTION_NAME_LEN = 16
# 16 actual spaces in binutils/ld/ldlang.c print_one_symbol
SYMBOL_NAME_SEP = 16

LINES_WITH_LINENO = IterWithPutBack[tuple[int, str]]


def consume_hex(s: str) -> tuple[int, str]:
    """
    Consumes a hex sequence (including 0x prefix and any leading space)
    returning the number and the rest of the string
    """
    if m := HEX_RE.match(s):
        return int(m[1], 0), s[m.end() :]
    raise ValueError("Failed to find hex number in", s)


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
        self.archives: dict[str, ArchiveMembers.Reference] = {}

    def feed(self, line: str, lines: LINES_WITH_LINENO):
        popped = None
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
                _lineno, line = popped = next(lines)
                reference = line[REFERENCE_ALIGN:]
            except StopIteration:
                return
        try:
            reference = ArchiveMembers.Reference.parse(reference)
            if reference is not None:
                self.archives[definition] = reference
        except:
            if popped is not None:
                lines.put_back(popped)
            raise


@dataclass(frozen=True)
class Symbol:
    name: str
    address: int
    wildcard: str


@dataclass(frozen=True)
class Section:
    _REFERENCE_RE = re.compile(r"^(?P<object>.*) \((?P<symbol>.*)\)$")
    name: str
    address: int
    size: int
    object: str
    fill: int = 0
    type: str = field(init=False, default="unknown")

    @property
    def start(self):
        """
        Return first byte of the section.
        """
        return self.address

    @property
    def stop(self):
        """
        Return first byte past the section, as in range or mem[start:stop] syntax.
        """
        return self.address + self.size + self.fill

    def __len__(self) -> int:
        return self.size + self.fill

    T = t.TypeVar("T", bound="Section")

    @classmethod
    def parse(cls: type[T], name: str, line: str, lines: LINES_WITH_LINENO) -> T:
        popped = None
        try:
            if len(name) >= SECTION_NAME_LEN - 1:
                _lineno, line = popped = next(lines)
            # Left pad indicates section type
            name = name.lstrip()
            # Try to demangle C++
            if mangleIdx := name.find("._Z"):
                mangleIdx += 1  # Swallow "."
                try:
                    name = name[:mangleIdx] + demangle(name[mangleIdx:])
                except InvalidName:
                    pass
            line = line[SECTION_NAME_LEN:]
            addr, line = consume_hex(line)
            size, line = consume_hex(line)
            object = line.lstrip()
            return cls(name.strip(), addr, size, object)
        except:
            if popped is not None:
                lines.put_back(popped)
            raise

    def pretty_name(self) -> str:
        return self.name.strip('"')


@dataclass(frozen=True)
class InputSection(Section):
    symbols: tuple[Symbol, ...] = ()
    type: str = field(init=False, default="input")

    def pretty_name(self, output: OutputSection | None = None) -> str:
        prefix = ""
        if output is not None:
            prefix = output.pretty_name() + "."
        noprefix = super().pretty_name().removeprefix(prefix)
        if noprefix:
            return noprefix.strip('"')
        else:
            path = Path(self.object).resolve()
            return path.name


@dataclass(frozen=True)
class OutputSection(Section):
    inputs: tuple[InputSection, ...] = ()
    type: str = field(init=False, default="output")


class Discards:
    """
    See binutils/ld/ldlang.c print_input_section (with is_discarded = true)
    """

    _REFERENCE_ALIGN = 30
    type: str = field(init=False, default="discard")

    def __init__(self) -> None:
        self.discards = {}

    def feed(self, line: str, lines: LINES_WITH_LINENO):
        if line is None or line == "":
            return
        elif line[0] == " " and (m := SECTION_NAME_RE.match(line[1:])):
            name = " " + m[1]  # For constant columns
            discard = DiscardedSection.parse(name, line, lines)
            self.discards[discard.name] = discard


class DiscardedSection(OutputSection):
    pass


@dataclass(frozen=True)
class Area:
    name: str
    origin: int
    length: int
    attributes: frozenset[str]
    type: str = field(init=False, default="area")

    @property
    def start(self):
        """
        Return first byte of the area.
        """
        return self.origin

    @property
    def stop(self):
        """
        Return first byte past the area, as in range or mem[start:stop] syntax.
        """
        return self.origin + self.length

    def __len__(self) -> int:
        return self.length

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

    def feed(self, line: str, _lines: LINES_WITH_LINENO):
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


@dataclass
class _WipInputSection:
    base: InputSection
    fill: int = 0
    symbols: list[Symbol] = field(default_factory=list)

    def freeze(self) -> InputSection:
        return replace(self.base, fill=self.fill, symbols=tuple(self.symbols))


@dataclass
class _WipOutputSection:
    base: OutputSection
    fill: int = 0
    inputs: list[_WipInputSection] = field(default_factory=list)

    def freeze(self) -> OutputSection:
        return replace(
            self.base,
            fill=self.fill,
            inputs=tuple(map(_WipInputSection.freeze, self.inputs)),
        )


class MemoryMap:

    def __init__(self) -> None:
        self.sections: list[_WipOutputSection] = []

    def feed(self, line: str, lines: LINES_WITH_LINENO):
        if line is None or line == "":
            return
        elif any(
            line.startswith(op)
            for op in ["START GROUP", "LOAD", "END GROUP", " FILL mask"]
        ):
            return
        elif line.startswith("/DISCARD/"):
            return
        elif m := SECTION_NAME_RE.match(line):
            self.new_output_section(m[1], line, lines)
        elif line[0] == " " and (m := SECTION_NAME_RE.match(line[1:])):
            name = " " + m[1]  # For constant columns
            self.new_input_section(name, line, lines)
        elif m := RELAXED_RE.match(line):
            size = int(m[1], 0)
            self.relaxed_symbol(size)
        elif len(line) > SECTION_NAME_LEN and line[:SECTION_NAME_LEN].isspace():
            rest = line[SECTION_NAME_LEN:]
            # Technically symbol names can contain any values (see
            # e.g. rust embeddonomicon debugging with symbols) -- but
            # this just implies that we can have symbols that look exactly
            # like assignments so it's impossible to know in all
            # cases. So we just use the presence of an equals to detech
            # an assignment.
            if " = " in rest:
                self.add_assignment(rest)
            elif rest[:SYMBOL_NAME_SEP].isspace():
                line = line[SECTION_NAME_LEN:]
                addr, rest = consume_hex(line)
                self.add_symbol(addr, rest)
        elif "(" in line:
            self.wildcard_section(line)
        elif line[:8] == " *fill* ":
            self.add_fill(line[8:])
        else:
            print("UNKNOWN       ", line)

    def new_output_section(self, name: str, line: str, lines: LINES_WITH_LINENO):
        "See binutils/ld/ldlang.c print_output_section_statement"
        try:
            self.sections.append(
                _WipOutputSection(OutputSection.parse(name, line, lines))
            )
        except ValueError:
            # Output sections without addresses are possible if they
            # haven't had anything put in them
            # The parse call puts back a line if it popped a line
            pass

    def wildcard_section(self, line: str):
        "See binutils/ld/ldlang.c print_wild_statement"
        self._wildcard = line.strip()

    def new_input_section(self, section: str, line: str, lines: LINES_WITH_LINENO):
        "See binutils/ld/ldlang.c print_intput_statement"
        assert (
            self.sections != []
        ), "Expecting an output section before an input section"
        input_sections = self.sections[-1].inputs
        input_sections.append(
            _WipInputSection(InputSection.parse(section, line, lines))
        )

    def relaxed_symbol(self, _size: int):
        "See binutils/ld/ldlang.c print_input_section"
        pass

    def add_assignment(self, _line: str):
        "See binutils/ld/ldlang.c print_one_symbol"
        # TODO: [addr], [unresolved], or *undef*
        # addr = int(line[:HEXDIGITS], 0)
        # +2 for [0x1234567] or [unresolved]
        # assignment = line[HEXDIGITS + 2 + ASSIGNMENT_NAME_SEP :]
        # print("Assignment    ", f"0x{addr:08X}", f"'{assignment}'")
        # Not used -- do nothing for now

    def add_symbol(self, addr: int, rest: str):
        "See binutils/ld/ldlang.c print_one_symbol"
        name = rest[SYMBOL_NAME_SEP:]
        assert self.sections != [], "Expecting an output section before a symbol"
        input_sections = self.sections[-1].inputs
        assert input_sections != [], "Expecting an input section before symbol"
        input_section = input_sections[-1]
        input_section.symbols.append(Symbol(name, addr, self._wildcard))

    def add_fill(self, rest: str):
        "See binutils/ld/ldlang.c print_padding_statement"
        _addr, rest = consume_hex(rest)
        size, _rest = consume_hex(rest)

        assert self.sections != [], "Expecting an output section before fill"
        output_section = self.sections[-1]
        output_section.fill += size

        if output_section.inputs != []:
            input_section = output_section.inputs[-1]
            input_section.fill += size


@dataclass
class MapFile:
    archive_members: dict[str, ArchiveMembers.Reference] = field(default_factory=dict)
    discards: dict[str, DiscardedSection] = field(default_factory=dict)
    memory_configuration: list[Area] = field(default_factory=list)
    memory_map: list[OutputSection] = field(default_factory=list)

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

        lines = IterWithPutBack(
            enumerate(line[:-1] for line in iter(file_like.readline, ""))
        )
        for _lineno, line in lines:
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
            memory_map=list(map(_WipOutputSection.freeze, memory_map.sections)),
        )

    def iter_area_sections(self) -> t.Iterator[tuple[Area, list[OutputSection]]]:
        """
        Group each section by area (e.g. FLASH/SRAM, depending on how it's
        defined in the linker script).
        """
        areas = IntervalTree(
            Interval(area.origin, area.stop, (area, []))
            for area in self.memory_configuration
            if area.attributes and len(area) > 0
        )
        for section in self.memory_map:
            for _start, _end, (area, area_sections) in areas[section]:
                area_sections.append(section)
        for _start, _end, (area, sections) in areas:
            yield area, sections
