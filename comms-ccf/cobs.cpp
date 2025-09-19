#include "cobs.hpp"

#if defined(DEBUG_COBS)
#include DEBUG_COBS
#else
#include "ndebug.hpp"
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
            DEBUG "op++ hdr? %s %d/%d/%ld char %02X ->" LOGLEVEL_ARGS,
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
            " hdr? %s %d/%d/%ld char %02X" END,
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

    bool Decoder::feed(uint8_t byte)
    {
        bool skip = runLengthWasMax && runLength == 0;
        if (byte == 0)
        {
            /// We reached an unescaped zero byte, our caller should
            /// already know we are at the end -- reset for decoding
            /// next frame
            runLength = 0;
            runLengthWasMax = true;
            /// And don't say skip, to emit the null and not process the
            /// next byte
            return true;
        }
        else if (runLength == 0)
        {
            debugf(DEBUG "runLength == 0 at byte %02X" END LOGLEVEL_ARGS, byte);
            runLength = byte;
            runLengthWasMax = runLength - 1 == maxRunLength;
        }
        --runLength;
        return !skip;
    }

    uint8_t Decoder::get(uint8_t byte) const
    {
        if (runLength == 0)
        {
            return 0;
        }
        return byte;
    }
};
