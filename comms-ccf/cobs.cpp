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
