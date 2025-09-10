import pydoc
import time
import typing as t
from inspect import Parameter, signature
from textwrap import indent

from cbor import dumps, loads
from transport import DEFAULT_TIMEOUT, Transport


class Chan:
    RPC = 0


class Rpc:
    def __init__(self, transport: Transport):
        self._transport = transport
        self._methods: dict[str, t.Callable[..., t.Any]] = {"schema": self.schema}
        self._doc = pydoc.TextDoc()

    async def __call__(self, n, args: t.Any, timeout=DEFAULT_TIMEOUT) -> t.Any:
        deadline = time.time() + timeout
        data = int.to_bytes(n) + dumps(args)
        await self._transport.send(Chan.RPC, data, timeout=timeout)
        timeout = deadline - time.time()
        if timeout > 0:
            chan, data = await self._transport.recv(timeout=timeout)
            assert chan == Chan.RPC
            function = data[0]
            data = data[1:]
            assert function == n, "Received response to a different function"
            return loads(data)
        else:
            raise TimeoutError("No data received over RPC in the given time")

    async def discover(self, timeout=DEFAULT_TIMEOUT):
        self._schema = await self(0, [], timeout=DEFAULT_TIMEOUT)
        assert isinstance(self._schema, list)
        for index, fun in enumerate(self._schema, start=1):
            self.add_function(index, *fun)

    def schema(self) -> list[list[str]]:
        "show the RPC schema"
        return self._schema

    def add_function(self, index, name, doc, ret, *args):
        # Using default arguments is a workaround to using `index`
        # by value not reference
        wrapper = lambda n=index: lambda *args: self(n, args)  # noqa: E731

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
