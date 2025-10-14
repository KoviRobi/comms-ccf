/**

# Iterator utility tools

This gives us some utility functions, to be able to use output iterators, fed
from the interrupt, into a buffer, instead of input iterators. Compare the
following using output iterator, from the ISR

```cpp
void isr()
{
    static CircularBuffer<uint8_t, MAX_PKT_SIZE> buf;
    static auto it =
        Split(0) >>=
        Transform(Cobs::Decoder::Out) >>=
        Transform(return std::back_inserter(buf));
}
```

*/

#include <stddef.h>
#include <stdio.h>

#include <iterator>
#include <ranges>

class End{};

template<typename Value, typename B, typename E>
class Split
{
public:
    class Iterator
    {
    public:
        using value_type = Value;
        using difference_type = ptrdiff_t;

        Iterator(Split * outer_) : outer(outer_) { }
        Iterator & operator*() const { return *this; }
        Iterator operator++(int) { auto tmp = *this; ++*this; return tmp; }
        const Iterator & operator=(const Value & value) const
        {
            outer->discard = value == outer->separator;
            if (!outer->discard)
            {
                *outer->begin_ = value;
            }
            return *this;
        }
        Iterator & operator++()
        {
            if (!outer->discard)
            {
                ++outer->begin_;
            }
            return *this;
        }

        bool operator!=(const End &) const
        {
            return *outer != outer->end() && !outer->discard;
        }

        Iterator & begin() { return *this; }
        End end() { return {}; }

    private:
        Split * outer;
    };

    using value_type = Iterator;
    using difference_type = ptrdiff_t;

    Split(Value && separator_, B begin, E end)
        : separator(separator_), begin_(begin), end_(end) { }

    template<std::ranges::viewable_range Rng>
    Split(Value && separator_, Rng && range)
        : separator(separator_),
          begin_(std::ranges::begin(std::forward<Rng>(range))),
          end_(std::ranges::end(std::forward<Rng>(range)))
    {
    }

    Iterator operator*() { return Iterator(this); }
    Split operator++(int) { auto tmp = *this; ++*this; return tmp; }
    Split & operator++()
    {
        if (!discard)
        {
            ++begin_;
        }
        return *this;
    }

    bool operator!=(const End &) const
    {
        return begin_ != end_;
    }

    Split & begin() { return *this; }
    End end() { return {}; }

private:
    Value separator;
    B begin_;
    E end_;
    bool discard = false;
};
static_assert(std::output_iterator<Split<char, char*, char*>, char>);
static_assert(
    std::output_iterator<Split<char, char*, char*>::Iterator, char>
);

template<typename Fn, typename B, typename E>
class TransformIterator
{
public:
    using value_type = typename std::iter_value_t<B>;
    using difference_type = typename std::iter_difference_t<B>;

    TransformIterator(Fn && fn_, B && b, E && e)
        : fn(fn_), begin_(b), end_(e) { }

    TransformIterator & operator*() const { return *this; }
    TransformIterator & operator++() { ++*begin_; return *this; }
    TransformIterator & operator++(int) { auto tmp = *this; ++*begin_; return tmp; }
    TransformIterator & operator=(const value_type & value)
    {
        *begin_ = fn(value);
    }

private:
    Fn fn;
    B begin_;
    E end_;
};

template<typename Fn>
class Transform
{
public:
    Transform(Fn && _fn) : fn(std::forward<Fn>(_fn)) { }

    template<typename B, typename E>
    TransformIterator<Fn, B, E> operator()(B && b, E && e)
    {
        return TransformIterator<Fn, B, E>(fn, std::forward<B>(b), std::forward<E>(e));
    }

private:
    Fn fn;
};
static_assert(
    std::output_iterator<TransformIterator<char(*)(char), char*, char*>, char>
);
