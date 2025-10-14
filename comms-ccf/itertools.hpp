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

class Transform
{
public:
};
