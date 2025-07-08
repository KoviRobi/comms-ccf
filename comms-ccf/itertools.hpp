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

template<size_t Size>
class Buffer
{
public:
    static_assert(std::popcount(Size) == 1, "Size must be a power of 2");

    using difference_type = ptrdiff_t;
    using value_type = uint8_t;
    value_type operator*() const;

    /// Forward iterator
    Buffer & operator++();
    Buffer operator++(int) { auto tmp = *this; ++*this; return tmp; }
    bool operator==(const Buffer &) const;

    /// Backwards to make it a bidirectional iterator
    Buffer & operator--();
    Buffer operator--(int) { auto tmp = *this; ++*this; return tmp; }

    /// Random access iterator
    Buffer & operator+=(int);
    Buffer & operator-=(int);
    Buffer operator+(difference_type) const;
    difference_type operator-(const Buffer &) const;
    Buffer operator-(difference_type) const;
    int operator<=>(const Buffer &) const;

    template<size_t _S>
    friend Buffer<_S> operator+(typename Buffer<_S>::difference_type, const Buffer<_S> &);

    value_type operator[](int) const;

private:
    std::array<value_type, Size> buf;
};

static_assert(std::random_access_iterator<Buffer<256>>);
