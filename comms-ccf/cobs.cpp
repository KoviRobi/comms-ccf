#include "cobs.hpp"

#if defined(DEBUG_COBS)
#include DEBUG_COBS
#else
extern int printf(const char *, ...);
/// Need an expression that contains parameters for parameter pack
/// Want to avoid linking printf at any optimisation, hence the constexpr if
/// Want an expression not a statement, hence the lambda wrapper
#define debugf(...) ([&]{ if constexpr (false) { printf(__VA_ARGS__); } }())
#endif

#include <stdint.h>

#include <span>

namespace Cobs
{
    uint8_t Encoder::findRunLength()
    {
        // Because we output `runLength + 1`, i.e. the pointer to the null not
        // the last non-null byte, the maximum is 254
        uint8_t runLength = 0;
        for (; runLength < maxRunLength && runLength < data.size(); ++runLength)
        {
            if (data[runLength] == 0)
            {
                break;
            }
        }
        return runLength;
    }

    Encoder::value_type Encoder::operator*() const
    {
        if (!runHeaderOutput)
        {
            // Pointer to the first zero byte, not the last non-zero byte
            return runLength + 1;
        }
        else
        {
            return data[runIndex];
        }
    }

    Encoder & Encoder::operator++()
    {
        debugf(
            "op++ hdr? %s %d/%d/%ld char %02X ->",
            runHeaderOutput ? "done" : "todo",
            runIndex,
            runLength,
            data.size(),
            data[runIndex]
        );
        if (!runHeaderOutput)
        {
            runHeaderOutput = true;
        }
        else if (runIndex < runLength)
        {
            ++runIndex;
        }
        // Not `else if`, need to recalculate before dereference
        if (runIndex == runLength)
        {
            if (runIndex < data.size())
            {
                if (runLength < maxRunLength)
                {
                    // Skip over zero byte
                    ++runIndex;
                }
                runHeaderOutput = false;
            }
            data = data.subspan(runIndex);
            runIndex = 0;
            runLength = findRunLength();
        }
        debugf(
            " hdr? %s %d/%d/%ld char %02X\n",
            runHeaderOutput ? "done" : "todo",
            runIndex,
            runLength,
            data.size(),
            data[runIndex]
        );
        return *this;
    }

    bool Encoder::operator!=(IteratorEnd) const
    {
        bool atEnd = (runIndex == data.size()) && runHeaderOutput;
        return !atEnd;
    }
};
