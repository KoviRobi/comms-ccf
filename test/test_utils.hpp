/*

# Simple test utils

Just a very basic set of macros for test assertions. Nothing fancy,
but enough for writing some quick tests

*/

#include <stddef.h>
#include <stdio.h>

#define assert(...)                                                          \
    if (!assert_((__VA_ARGS__), __FILE__, __func__, __LINE__, #__VA_ARGS__)) \
    {                                                                        \
        return false;                                                        \
    }

#define expect(...)                                                          \
    ok = ok && assert_((__VA_ARGS__), __FILE__, __func__, __LINE__, #__VA_ARGS__)

static bool assert_(bool expr, const char * file, const char * function, size_t line, const char * assertion)
{
    if (!expr)
    {
        fprintf(stderr,
            "%s:%zu %s error: assertion %s failed\n",
            file, line, function, assertion);
        return false;
    }
    return true;
}
