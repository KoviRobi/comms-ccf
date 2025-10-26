"""
Implements the Segger RTT Telnet protocol, see
https://kb.segger.com/J-Link_RTT_TELNET_Channel
"""

import asyncio
import os
import re
import sys
from argparse import ArgumentParser
from contextlib import asynccontextmanager
from pathlib import Path

import elftools.elf.sections as elf_sections
from elftools.elf.elffile import ELFFile

CURSOR_UP, FG_RESET, FG_GREEN, FG_YELLOW, FG_BLUE = [""] * 5
if os.isatty(1):
    CURSOR_UP = "\x1b[A"
    FG_RESET, FG_GREEN, FG_YELLOW, FG_BLUE = [f"\x1b[{d}m" for d in [0, 32, 33, 34]]

SEGGER_RTT_HELLO_RE = re.compile(
    r"SEGGER J-Link (?P<sw_version>V[^\r\n]{1,20}) - Real time terminal output\r?\n"
    r"SEGGER (?P<hw_version>[^\r\n]{1,40}), SN=(?P<hw_serial>.{1,20})\r?\n"
    r"Process: (?P<process>[^\r\n]{1,20})\r?\n",
)
# Rough upper limit for RE match, regex string size plus sum of all
# numbers in regex
SEGGER_RTT_HELLO_RE_MAX = len(SEGGER_RTT_HELLO_RE.pattern) + sum(
    map(int, re.findall(r"\d+", SEGGER_RTT_HELLO_RE.pattern))
)


async def get_rtt_address(target_elf: Path) -> int:
    """
    Finds the _SEGGER_RTT symbol address in an ELF file
    """
    while True:
        try:
            with target_elf.open("rb") as elf_fp, ELFFile(elf_fp) as elf:
                symtab = elf.get_section_by_name(".symtab")
                assert isinstance(symtab, elf_sections.SymbolTableSection)
                rtts = symtab.get_symbol_by_name("_SEGGER_RTT")
                if not isinstance(rtts, list):
                    print("No RTT blocks (_SEGGER_RTT symbols) found in the ELF")
                    print("Did you enable SEGGER RTT?")
                    sys.exit(1)
                assert rtts and len(rtts) == 1, "Only expecting one RTT block"
                rtt = rtts[0]
                return rtt["st_value"]
        except FileNotFoundError:
            print("Missing file", target_elf, "assuming it is being (re)built")
            await asyncio.sleep(1)


async def get_rtt_config(elf_or_address: None | Path) -> str:
    """
    Gets the RTT address from a file or a string
    """
    if elf_or_address is None:
        pass
    elif elf_or_address.exists():
        return f"SetRTTAddr;{hex(await get_rtt_address(elf_or_address))};"
    else:
        try:
            start, end = str(elf_or_address).split("+")
            return f"SetRTTSearchRanges;{hex(int(start, 0))} {hex(int(end, 0))};"
        except ValueError:
            return f"SetRTTAddr;{hex(int(elf_or_address.name, 0))};"


async def segger_rtt_client(
    host: str, port: int, channel: int, elf_or_address: Path | None
) -> tuple[asyncio.StreamReader, asyncio.StreamWriter]:
    """
    Handles a single connection, returning when the connection is closed.
    """

    config_str = f"$$SEGGER_TELNET_ConfigStr=RTTCh;{channel};"
    if elf_or_address is not None:
        config_str += await get_rtt_config(elf_or_address)
    config_str += "$$"

    start_wallclock = asyncio.get_running_loop().time()
    async with asyncio.timeout(0.1):
        reader, writer = await asyncio.open_connection(host, port)

    writer.write(config_str.encode())
    async with asyncio.timeout(0.1):
        await writer.drain()

    end_wallclock = asyncio.get_running_loop().time()
    if end_wallclock - start_wallclock > 0.1:
        raise TimeoutError("Failed to send RTT config within time")

    return reader, writer


async def greet(reader: asyncio.StreamReader) -> re.Match:
    buffer = ""
    while True:
        try:
            data = await reader.readuntil(b"\n")
            buffer += data.decode("ascii", "ignore")
            # This is algorithmically inefficient, but python has no way of
            # feeding the RE state machine -- hence trimming the buffer
            buffer = buffer[-SEGGER_RTT_HELLO_RE_MAX:]
            if match := SEGGER_RTT_HELLO_RE.search(buffer):
                return match
        except asyncio.IncompleteReadError:
            pass


def command_parser(parser_or_subparser: ArgumentParser) -> ArgumentParser:
    parser_or_subparser.add_argument(
        "--elf",
        type=Path,
        help="ELF file with _SEGGER_RTT symbol, or `<address>`, or `<address>+<length>` to search",
    )
    parser_or_subparser.add_argument(
        "--host", default="localhost", help="Host to connect to"
    )
    parser_or_subparser.add_argument(
        "--port", default=19021, type=int, help="Port to connect to"
    )
    return parser_or_subparser


@asynccontextmanager
async def command(args):
    while True:
        try:
            rx, tx = await segger_rtt_client(args.host, args.port, 0, args.elf)
            break
        except TimeoutError as e:
            print(f"{FG_YELLOW}Warning: {e}, restarting{FG_RESET}")

    async with asyncio.timeout(0.1):
        header = await greet(rx)

    print(f"{FG_BLUE}Segger {header['process']} {header['sw_version']}")
    print(f"{header['hw_version']} serial {header['hw_serial']}{FG_RESET}")
    yield rx, tx
    rx.feed_eof()
    tx.close()
