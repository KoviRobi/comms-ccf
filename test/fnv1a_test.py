import pytest
from fnvhash import fnv1a_32
from conftest import LibFnv1a

@pytest.mark.parametrize(
    argnames="data",
    argvalues=[b"", b"1234", bytes(range(255))]
)
def test_encode(libfnv1a: LibFnv1a, data):
    encoded = libfnv1a.hash(data)
    assert encoded == fnv1a_32(data)
