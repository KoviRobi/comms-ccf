"""
Simple hex-dump which is a mirror of the C++ hexdump in test/hexdump.hpp
"""

import sys


def hexdump(data: bytes, prefix="") -> str:
    ret = ""
    for line in range(0, len(data), 16):
        ret += f"{prefix}{line:04X}: "
        for col in range(0, 16):
            # fmt: off
            if    col      == 0: pass          # noqa
            elif (col & 7) == 0: ret += "    " # noqa
            elif (col & 3) == 0: ret += "  "   # noqa
            elif (col & 1) == 0: ret += " "    # noqa
            # fmt: on
            pos = line + col
            if pos < len(data):
                ret += f"{data[pos]:02X}"
            else:
                ret += "__"
        ret += "    "
        for col in range(0, 16):
            pos = line + col
            if pos < len(data):
                c = chr(data[pos])
                if str.isprintable(c):
                    ret += c
                else:
                    ret += "."
            else:
                break
        ret += "\n"
    return ret


def main():
    while buf := sys.stdin.buffer.read(256):
        print(hexdump(buf))


if __name__ == "__main__":
    main()
