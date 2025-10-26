/**
\file
\brief Remote procedure support.

# Remote Procedure Call (RPC)

We can have remote code execution very simply: read a function pointer,
then a list of arguments (e.g. CBOR encoded), and jump to that pointer
with the arguments on the stack. This is tedious and fragile, however.

The tedium we can help by using e.g. Python to do some of the sending
data, and the fragility we can help by defining at compile-time which
functions we want to be exposed, and also doing type checking on those
argumets.

We can further help the tedium and safety by transmitting a schema that
the endpoint supports. This way we can do checking at the Python level
even before we send data out.

Using template metaprogramming, we can also have type safety in C++.

#### `INLINE_VTABLE` {#INLINE_VTABLE}

This is just an experiment to see if we can remove one layer of pointer
chasing (object → vtable → function pointer becomes object → function
pointer)

*/

#pragma once

#include "cbor.hpp"
#include "comptime_str.hpp"

#if defined(DEBUG_RPC)
#include DEBUG_RPC
#else
#include "ndebug.hpp"
#endif

#include <stddef.h>
#include <stdint.h>

#include <array>
#include <functional>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>

#if defined(INLINE_VTABLE)
#define self (this_)
#define self_arg(type) const type & this_,
#define self_app(value) value,
#define prototype(ret, name, args) const ret (*name) args
#define definition(ret, name, args) static ret name args
#else
#define self (*this)
#define self_arg(type) /* implicit */
#define self_app(value) /* implicit */
#define prototype(ret, name, args) virtual ret name args const = 0
#define definition(ret, name, args) ret name args const override
#endif

#define LITERAL_COMPTIME_STRING(name, value) const decltype(CompTimeString{value}) name{value}
template<typename T>
struct Type { static constexpr CompTimeString python = "Any"; };

template<std::integral I>
struct Type<I> { static constexpr CompTimeString python = "int"; };

template<>
struct Type<std::string_view> { static constexpr CompTimeString python = "str"; };

template<size_t extent>
struct Type<std::span<uint8_t, extent>> { static constexpr CompTimeString python = "bytes"; };

template<>
struct Type<std::tuple<>> { static constexpr CompTimeString python{"tuple[()]"}; };
template<typename... Ts>
struct Type<std::tuple<Ts...>>
{
    constexpr static CompTimeString python{
        CompTimeString{"tuple["} +
        (..., Type<Ts>::python) +
        CompTimeString{"]"}
    };
};

class NonTemplatedCall
{
public:
    /// Encodes the schema for the function into `seq`.
    prototype(bool, schema, (self_arg(NonTemplatedCall) Cbor::Sequence<Cbor::Major::Array> & seq));
    /// Calls this function with the encoded args in `args` and encodes
    /// the return into `ret`.
    prototype(bool, call, (self_arg(NonTemplatedCall) std::span<uint8_t> & args, std::span<uint8_t> & ret));
};

template<typename Ret, typename... Args>
class Call : public NonTemplatedCall
{
public:
    using Fun = Ret (*)(Args...);
    using ArgsTup = std::tuple<Args...>;
    using Return = Ret;

    Call(
        const char * name_,
        const char * doc_,
        std::array<const char *, sizeof...(Args)> argNames_,
        Fun ptr_)
        :
#if defined(INLINE_VTABLE)
          NonTemplatedCall
          {
              // Note: Function arguments are normally contravariant
              // but in this case the `this_` are const so it is safe
              // (and all other arguments match).
              .schema = reinterpret_cast<decltype(NonTemplatedCall::schema)>(reinterpret_cast<void *>(&schema)),
              .call = reinterpret_cast<decltype(NonTemplatedCall::call)>(reinterpret_cast<void *>(&call)),
          },
#endif
          name(name_),
          doc(doc_),
          argNames(argNames_),
          ptr(ptr_) { }

    definition(bool, schema, (self_arg(Call) Cbor::Sequence<Cbor::Major::Array> & seq))
    {
        Cbor::Sequence<Cbor::Major::Array> subseq(seq, 3 + 2 * sizeof...(Args));
        return
            subseq.encode(std::string_view(self.name)) &&
            subseq.encode(std::string_view(self.doc)) &&
            subseq.encode(static_cast<std::string_view>(Type<Return>::python)) &&
            [&]<size_t... Idx>(std::index_sequence<Idx...>)
            {
                return (
                    (
                        subseq.encode(std::string_view(self.argNames[Idx])) &&
                        subseq.encode(static_cast<std::string_view>(
                            Type<std::tuple_element_t<Idx, ArgsTup>>::python))
                    ) &&
                    ...
                );
            }(std::index_sequence_for<Args...>{}) &&
            subseq.as_expected();
    }

    definition(bool, call, (self_arg(Call) std::span<uint8_t> & args, std::span<uint8_t> & ret))
    {
        if (self.ptr == nullptr)
        {
            debugf(WARN "function ptr is null, ignoring call" END LOGLEVEL_ARGS);
            return false;
        }
        debugf(DEBUG "decoding" LOGLEVEL_ARGS);
        for (const auto byte : args)
        {
            debugf(" %02X", byte);
        }
        debugf(END);
        auto argsTup = Cbor::Cbor<ArgsTup>::decode(args);
        if (argsTup)
        {
            debugf(DEBUG "function is %p" END LOGLEVEL_ARGS, self.ptr);
            auto retVal = Cbor::WrapVoid<Ret, Cbor::Undefined>{self.ptr, *argsTup};
            return Cbor::Cbor<Ret>::encode(retVal.value, ret);
        }
        return false;
    }
private:
    const char * const name;
    const char * const doc;
    const std::array<const char *, sizeof...(Args)> argNames;
    const Fun ptr;
};

template<typename... Calls>
class Rpc
{
public:
    Rpc(Calls && ... calls_)
    : tuple{std::forward<Calls>(calls_)...},
      calls{
          [&]<size_t... Idx>(std::index_sequence<Idx...>)
          {
              return std::array<const std::reference_wrapper<const NonTemplatedCall>, sizeof...(Calls)>{
                  (std::reference_wrapper<const NonTemplatedCall>{
                  *static_cast<const NonTemplatedCall *>(&std::get<Idx>(tuple))})...
              };
          }(std::index_sequence_for<Calls...>{})
      } { }

    /// Encode the schema for all the RPC functions onto buf.
    bool schema(std::span<uint8_t> & buf) const
    {
        Cbor::Sequence<Cbor::Major::Array> seq(buf, sizeof...(Calls));
        for (const auto & c : calls)
        {
            const auto & call = c.get();
            if (!call.schema(self_app(call) seq))
            {
                debugf(WARN "Schema failed to encode (buf size %zu)" END LOGLEVEL_ARGS, buf.size());
                return false;
            }
        }
        return seq.as_expected();
    }

    /// Calls the given RPC function (by index n) using the encoded
    /// arguments in `args` and encodes the result into `ret`.
    bool call(size_t n, std::span<uint8_t> & args, std::span<uint8_t> & ret) const
    {
        if (n == 0)
        {
            return schema(ret);
        }
        else if (n < sizeof...(Calls) + 1)
        {
            const auto & call = calls[n-1].get();
            return call.call(self_app(call) args, ret);
        }
        else
        {
            debugf(WARN "Tried to call function %zu but max is %zu" END LOGLEVEL_ARGS, n, sizeof...(Calls));
            return false;
        }
    }

    const std::tuple<const Calls...> tuple;
    const std::array<const std::reference_wrapper<const NonTemplatedCall>, sizeof...(Calls)> calls;
};

// This is a header, undefine the debugf macro
#include "debug_end.hpp"
