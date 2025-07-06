from __future__ import annotations

from collections import deque
from ctypes import (
    CDLL,
    CFUNCTYPE,
    POINTER,
    Structure,
    c_bool,
    c_char,
    c_size_t,
    c_ubyte,
    c_uint32,
    c_void_p,
    cast,
    cdll,
    create_string_buffer,
)
from pathlib import Path

import pytest
from cobs import cobs

# Note: c_char_p requires null-termination
c_bytes_p = POINTER(c_char)
c_bytes_pp = POINTER(c_bytes_p)
c_size_p = POINTER(c_size_t)


def pointerValue(pointer) -> int:
    return cast(pointer or 0, c_void_p).value or 0


def pytest_addoption(parser):
    parser.addoption("--libcobs", action="store", type=Path)


class StrStructure(Structure):
    def __repr__(self) -> str:
        return (
            type(self).__name__
            + "(\n    "
            + "\n    ".join(
                f"{field[0]}: {getattr(self, field[0])}" for field in self._fields_
            )
            + "\n)\n"
        )


class CobsEncoder(StrStructure):
    _fields_ = [
        ("data", c_bytes_p),
        ("size", c_size_t),
        ("runLength", c_ubyte),
        ("runIndex", c_ubyte),
        ("runHeaderOutput", c_bool),
    ]


class CobsDecoder(StrStructure):
    _fields_ = [
        ("runLength", c_ubyte),
        ("remaining", c_bool),
    ]


CobsEncoder_p = POINTER(CobsEncoder)
CobsDecoder_p = POINTER(CobsDecoder)


class LibCobs:
    def __init__(self, lib_path: Path):
        self.lib = cdll.LoadLibrary(str(lib_path))

        self.lib.cobsEncoderNew.argtypes = [c_bytes_p, c_size_t]
        self.lib.cobsEncoderNew.restype = CobsEncoder_p
        self.cobsEncoderNew = self.lib.cobsEncoderNew

        self.lib.cobsEncoderDelete.argtypes = [CobsEncoder_p]
        self.lib.cobsEncoderDelete.restype = None
        self.cobsEncoderDelete = self.lib.cobsEncoderDelete

        self.lib.cobsEncode.argtypes = [CobsEncoder_p, c_bytes_p, c_size_t]
        self.lib.cobsEncode.restype = c_size_t
        self.cobsEncode = self.lib.cobsEncode

        self.lib.cobsDecoderNew.argtypes = []
        self.lib.cobsDecoderNew.restype = CobsDecoder_p
        self.cobsDecoderNew = self.lib.cobsDecoderNew

        self.lib.cobsDecoderDelete.argtypes = [CobsDecoder_p]
        self.lib.cobsDecoderDelete.restype = None
        self.cobsDecoderDelete = self.lib.cobsDecoderDelete

        self.lib.cobsDecode.argtypes = [
            CobsDecoder_p,
            c_bytes_p,
            c_size_t,
            c_bytes_p,
            c_size_t,
        ]
        self.lib.cobsDecode.restype = c_size_t
        self.cobsDecode = self.lib.cobsDecode

    def encode(self, data: bytes) -> bytes:
        state = self.cobsEncoderNew(data, len(data))
        out_len = cobs.max_encoded_length(len(data))
        buf = create_string_buffer(out_len)
        try:
            enc_len = self.cobsEncode(state, buf, len(buf))
        finally:
            self.cobsEncoderDelete(state)
        return buf.raw[:enc_len]

    def decode(self, data: bytes) -> bytes:
        out_len = len(data)
        buf = create_string_buffer(out_len)
        state = self.cobsDecoderNew()
        try:
            dec_len = self.cobsDecode(state, data, len(data), buf, len(buf))
        finally:
            self.cobsDecoderDelete(state)
        return buf.raw[:dec_len]


@pytest.fixture(scope="session")
def libcobs(request):
    opt = request.config.getoption("--libcobs")
    if not opt:
        pytest.skip()
    return LibCobs(opt)
