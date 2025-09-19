#pragma once

#ifdef __cplusplus
#include <ccf.hpp>

#include "mutex.hpp"
#endif

#ifdef __cplusplus
extern "C"
{

extern Mutex<Ccf<{
    .rxBufSize = 256,
    .txBufSize = 256,
    .maxPktSize = 255,
}>> ccf;
#endif

/// Implement this in FreeRTOS for transport TX
extern void commsCcfTx(uint8_t byte);

/// Call this when you know there is data to TX or TX has finished
void commsCcfTxNext();

/// Add this to the transport RX callback
void commsCcfRx(uint8_t byte);

/// Use this to start the comms task
void commsCcfStartTasks(void);

#ifdef __cplusplus
};
#endif
