/**
\file
\brief Config for the Comms-CCF use.
*/
#pragma once

#include "mutex.hpp"

#include <ccf.hpp>

extern Mutex<Ccf<{
    .rxBufSize = 256,
    .txBufSize = 256,
    .maxPktSize = 255,
}>> ccf;
