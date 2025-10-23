"""
Discover RPC functions and add the documentation/type hints from the
schema.
"""

import pydoc
import time
import typing as t
from inspect import Parameter, signature
from random import randint
from textwrap import indent

from cbor import dumps, loads

from comms_ccf.channel import Channel, Channels
from comms_ccf.transport import DEFAULT_TIMEOUT


class Rpc:
    def __init__(self, channels: Channels, seqNo: int = randint(0, 0xFF)):
        self._channels = channels
        self._methods: dict[str, t.Callable[..., t.Any]] = {"schema": self.schema}
        self._doc = pydoc.TextDoc()
        self._seqNo = seqNo

    async def __call__(
        self, n: int, args: t.Any, timeout: float = DEFAULT_TIMEOUT
    ) -> t.Any:
        deadline = time.time() + timeout
        seqNo = self._seqNo
        self._seqNo = (self._seqNo + 1) & 0xFF
        data = int.to_bytes(seqNo) + int.to_bytes(n) + dumps(args)
        await self._channels.send(Channel.RPC, data, timeout=timeout)
        while True:
            timeout = deadline - time.time()
            data = await self._channels.recv(Channel.RPC, timeout=timeout)
            if data[0] != seqNo:
                continue
            function = data[1]
            data = data[2:]
            assert function == n, "Received response to a different function"
            return loads(data)

    async def discover(self, timeout: float = DEFAULT_TIMEOUT):
        self._channels.open_channel(Channel.RPC)
        self._schema = await self(0, [], timeout=timeout)
        print("Schema", self._schema)
        assert isinstance(self._schema, list), "Bad schema"
        for index, fun in enumerate(self._schema, start=1):
            self.add_function(index, *fun)

    def schema(self) -> list[list[str]]:
        "show the RPC schema"
        return self._schema

    def add_function(self, index, name, doc, ret, *args):
        # Using default arguments is a workaround to using `index`
        # by value not reference
        def wrapper(n=index):
            return lambda *args, timeout: self(n, args, timeout=timeout)

        sig = [
            Parameter(
                args[i], kind=Parameter.POSITIONAL_OR_KEYWORD, annotation=args[i + 1]
            )
            for i in range(0, len(args), 2)
        ]

        call = wrapper()
        call.__name__ = name
        call.__doc__ = doc
        call.__module__ = __name__
        call.__qualname__ = f"{Rpc.__qualname__}.{name}"
        call.__signature__ = signature(call).replace(
            parameters=sig, return_annotation=ret
        )
        self._methods[name] = call
        setattr(self, name, call)
        print(f"Discovered {call.__name__}{call.__signature__}")

    def methods(self) -> dict[str, t.Callable[..., object]]:
        return self._methods

    def help(self, thing: t.Optional[object] = None):
        print("\n")
        if thing is not None:
            print(indent(self._doc.document(thing), "| "))
        else:
            for method in self._methods.values():
                print(indent(self._doc.document(method), "| "))
