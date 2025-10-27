#include "ccf-config.hpp"

#include "mutex.hpp"

#include <ccf.hpp>

Mutex<Ccf<{
    .rxBufSize = 256,
    .txBufSize = 256,
    .maxPktSize = 255,
}>> ccf;
