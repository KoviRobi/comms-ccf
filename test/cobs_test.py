import pytest
from cobs import cobs
from conftest import LibCobs, create_string_buffer
from hypothesis import given
from hypothesis import strategies as st


@given(st.binary())
def test_encode(libcobs: LibCobs, data):
    encoded = libcobs.encode(data)
    assert encoded == cobs.encode(data)


@pytest.mark.parametrize(
    "data,out_len",
    [
        (b"", 1),
        (b"a", 2),
        (b"\0" * 254, 255),
        (b"\0" * 255, 256),
        (b"\1" * 254, 255),
        (b"\1" * 255, 257),
    ],
)
def test_edge_case_enough(libcobs: LibCobs, data, out_len):
    state = libcobs.cobsEncoderNew(data, len(data))
    buf = create_string_buffer(out_len)
    try:
        enc_len = libcobs.cobsEncode(state, buf, len(buf))
    finally:
        libcobs.cobsEncoderDelete(state)
    assert buf.raw[:enc_len] == cobs.encode(data)


@given(st.binary())
def test_edge_case_bigger_buffer(libcobs: LibCobs, data):
    state = libcobs.cobsEncoderNew(data, len(data))
    out_len = cobs.max_encoded_length(len(data))
    buf = create_string_buffer(out_len * 2)
    try:
        enc_len = libcobs.cobsEncode(state, buf, len(buf))
    finally:
        libcobs.cobsEncoderDelete(state)
    assert buf.raw[:enc_len] == cobs.encode(data)
