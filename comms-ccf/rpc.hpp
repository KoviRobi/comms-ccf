/**

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

*/

#pragma once

#if defined(DEBUG_RPC)
#include DEBUG_RPC
#else
extern int printf(const char *, ...);
/// Need an expression that contains parameters for parameter pack
/// Want to avoid linking printf at any optimisation, hence the constexpr if
/// Want an expression not a statement, hence the lambda wrapper
#define debugf(...) ([&]{ if constexpr (false) { printf(__VA_ARGS__); } }())
#endif

#include "cbor.hpp"

#include <stddef.h>
#include <stdint.h>

#include <array>
#include <functional>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>

template<typename T>
constexpr const char * pyType = "unknown";

template<std::integral I>
constexpr const char * pyType<I> = "int";

template<>
constexpr const char * pyType<std::string_view> = "str";

template<size_t extent>
constexpr const char * pyType<std::span<uint8_t, extent>> = "bytes";

class AbstractCall
{
public:
    virtual bool schema(Cbor::Sequence<Cbor::Major::Array> & seq) const = 0;
    virtual bool call(std::span<uint8_t> & args, std::span<uint8_t> & ret) const = 0;
};

template<typename Ret, typename... Args>
class Call : public AbstractCall
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
        : name(name_),
          doc(doc_),
          argNames(argNames_),
          ptr(ptr_) { }

    bool schema(Cbor::Sequence<Cbor::Major::Array> & seq) const override
    {
        Cbor::Sequence<Cbor::Major::Array> subseq(seq, 3 + 2 * sizeof...(Args));
        return
            subseq.encode(std::string_view(name)) &&
            subseq.encode(std::string_view(doc)) &&
            subseq.encode(std::string_view(pyType<Return>)) &&
            [&]<size_t... Idx>(std::index_sequence<Idx...>)
            {
                return (
                    (
                        subseq.encode(std::string_view(argNames[Idx])) &&
                        subseq.encode(std::string_view(pyType<std::tuple_element_t<Idx, ArgsTup>>))
                    ) &&
                    ...
                );
            }(std::index_sequence_for<Args...>{}) &&
            subseq.as_expected();
    }

    bool call(std::span<uint8_t> & args, std::span<uint8_t> & ret) const override
    {
        if (ptr == nullptr)
        {
            debugf("function ptr is null, ignoring call\n");
            return false;
        }
        debugf("decoding");
        for (const auto byte : args)
        {
            debugf(" %02X", byte);
        }
        debugf("\n");
        auto argsTup = Cbor::Cbor<ArgsTup>::decode(args);
        if (argsTup)
        {
            debugf("function is %p\n", ptr);
            Ret retVal = std::apply(ptr, *argsTup);
            debugf("call returned\n");
            return Cbor::Cbor<Ret>::encode(retVal, ret);
        }
        return false;
    }
private:
    const char * name;
    const char * doc;
    std::array<const char *, sizeof...(Args)> argNames;
    Fun ptr;
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
              return std::array{
                  (std::reference_wrapper<AbstractCall>{
                  *static_cast<AbstractCall *>(&std::get<Idx>(tuple))})...
              };
          }(std::index_sequence_for<Calls...>{})
      } { }

    bool schema(std::span<uint8_t> & buf) const
    {
        Cbor::Sequence<Cbor::Major::Array> seq(buf, sizeof...(Calls));
        for (auto & c : calls)
        {
            if (!c.get().schema(seq))
            {
                return false;
            }
        }
        return seq.as_expected();
    }

    bool call(size_t n, std::span<uint8_t> & args, std::span<uint8_t> & ret) const
    {
        return
            n == 0
            ? schema(ret)
            : n-1 < sizeof...(Calls) && calls[n-1].get().call(args, ret);
    }

    std::tuple<Calls...> tuple;
    std::array<std::reference_wrapper<AbstractCall>, sizeof...(Calls)> calls;
};

// This is a header, undefine the debugf macro
#undef debugf
