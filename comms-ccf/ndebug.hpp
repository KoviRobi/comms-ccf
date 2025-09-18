/**

The common debug ignore macros. If including this in a header, don't
forget to:

```
#include "debug_end.hpp"
```

*/

/// Note: no `#pragma once` because we `#undefine debugf` at the ends
/// of headers to be able to use debugf inside header code

#ifndef debugf

extern void ignoreVarArgs(...);

/// Need an expression that contains parameters for parameter pack
/// Want to avoid linking printf at any optimisation, hence the constexpr if
/// Want an expression not a statement, hence the lambda wrapper
#define debugf(...) ([&]{ if constexpr (false) { ignoreVarArgs(__VA_ARGS__); } }())

/// Start of debug printing format arguments -- always specify when
/// using DEBUG/INFO/WARN/ERROR
#ifndef START
#define START 0
#endif

/// Debug levels at the start of debugf
#ifndef DEBUG
#define DEBUG
#endif

#ifndef INFO
#define INFO
#endif

#ifndef WARN
#define WARN
#endif

#ifndef ERROR
#define ERROR
#endif

/// Terminator of log message at the end
#ifndef END
#define END
#endif

#else
#error debugf already defined, did you forget to `#include "debug_end.hpp"` at the end of a header?
#endif
