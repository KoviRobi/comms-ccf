/**

# Iterator utility tools

This contains some tools for working with iterators.

## Buffer up to given size

This allows us to buffer the input up to the given size, so that we can read it
back.

*/

#include <stddef.h>
#include <stdint.h>

#include <array>
#include <iterator>
#include <ranges>

template<typename Range, size_t Size>
class Buffer : std::ranges::view_interface<Buffer<Range, Size>>
{
public:
    using difference_type = ptrdiff_t;
    using value_type = uint8_t;

    class Iterator
    {
    public:
        using difference_type = Buffer::difference_type;
        using value_type = Buffer::value_type;
        value_type operator*() const;

        /// Forward iterator
        Iterator & operator++();
        Iterator operator++(int) { auto tmp = *this; ++*this; return tmp; }
        bool operator==(const Iterator &) const;

        /// Backwards to make it a bidirectional iterator
        Iterator & operator--();
        Iterator operator--(int) { auto tmp = *this; --*this; return tmp; }

        /// Random access iterator
        Iterator & operator+=(int);
        Iterator & operator-=(int);
        Iterator operator+(difference_type) const;
        difference_type operator-(const Iterator &) const;
        Iterator operator-(difference_type) const;
        int operator<=>(const Iterator &) const;

        // The warning is for porting old code, this is not relevant here.
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-template-friend"
        friend Iterator operator+(const Iterator::difference_type, const Iterator &);
#pragma GCC diagnostic pop

        value_type operator[](int) const;

    private:
        const Buffer * buf = nullptr;
        difference_type index = 0;
    };

    // template<std::ranges::viewable_range Arg>
    template<typename Arg>
    Buffer(Arg && arg) : range(std::forward<Arg>(arg)) { }

    Iterator begin() const
    {
        if (index < Size)
        {
            return Iterator(this, 0);
        }
        else
        {
            return Iterator(this, (index + 1) % Size);
        }
    }
    Iterator end() const { return Iterator(this, index % Size); }

    Buffer & operator++() { index++; return *this; }
    Buffer operator++(int) { auto tmp = *this; ++*this; return tmp; }
    value_type & operator*() { return buf[index]; }

private:
    Range range;
    difference_type index = 0;
    std::array<value_type, Size> buf = {};
};

/*
template<std::ranges::viewable_range Range, size_t Size>
Buffer<Range, Size>(Range && range) -> Buffer<std::ranges::views::all_t<Range>, Size>;
*/

static_assert(std::output_iterator<Buffer<uint8_t*, 256>, uint8_t>);
static_assert(std::random_access_iterator<Buffer<uint8_t*, 256>::Iterator>);
