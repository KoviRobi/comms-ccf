#pragma once

#include <stdint.h>

#ifdef __cplusplus
extern "C"
{
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
