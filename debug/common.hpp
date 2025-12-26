/**
\file

Colorize debug levels

*/

#ifndef debugf

#define BLACK      30
#define RED        31
#define GREEN      32
#define YELLOW     33
#define BLUE       34
#define MAGENTA    35
#define CYAN       36
#define WHITE      37
#define BRBLACK    90
#define BRRED      91
#define BRGREEN    92
#define BRYELLOW   93
#define BRBLUE     94
#define BRMAGENTA  95
#define BRCYAN     96
#define BRWHITE    97

#define _STR(x) #x
#define COL(c)    "\x1b[" _STR(c) "m"
#define RESET     "\x1b[0m"

#ifndef STRIP_FILE_DEFINED
#ifndef STRIP_ROOT
/// Return the last three filepath components
constexpr const char * strip___FILE__(const char * file)
{
    unsigned slashes[3] = {0u};
    unsigned i = 0u, slash = 0u;
    for (; file[i]; ++i)
    {
        if (file[i] == '/')
        {
            slashes[slash++ % (sizeof(slashes) / sizeof(slashes[0]))] = i + 1;
        }
    }
    return &file[slashes[slash % (sizeof(slashes) / sizeof(slashes[0]))]];
}
#define STRIP_FILE_DEFINED
#else
/// Strip a predefined prefix given by STRIP_ROOT
constexpr const char * strip___FILE__(const char * file)
{
    const char * root = STRIP_ROOT;
    while (*file && *root && *file == *root)
    {
        ++file;
        ++root;
    }
    return file;
}
#define STRIP_FILE_DEFINED
#endif
#endif

/// Start of debug log
#define LOGLEVEL_ARGS , strip___FILE__(__FILE__), __LINE__
/// Debug levels at the start of debugf
#define DEBUG COL(BRBLACK)  "%s:%u debug: "
#define INFO  COL(CYAN)     "%s:%u info:  "
#define WARN  COL(YELLOW)   "%s:%u warn:  "
#define ERROR COL(RED)      "%s:%u error: "
/// Terminator of log message at the end
#define END RESET "\n"

#else
#error debugf already defined, did you forget to `#include "debug_end.hpp"` at the end of a header?
#endif
