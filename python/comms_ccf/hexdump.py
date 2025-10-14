"""
Simple hex-dump which is a mirror of the C++ hexdump in test/hexdump.hpp
"""

def hexdump(data: bytes, prefix="") -> str:
    ret = ""
    for line in range(0, len(data), 16):
        ret += f"{prefix}{line:04X}: "
        for col in range(0, 16):
            if col == 0: pass
            elif (col & 7) == 0: ret += "    "
            elif (col & 3) == 0: ret += "  "
            elif (col & 1) == 0: ret += " "
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
