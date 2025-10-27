#pragma once

#ifdef __cplusplus
extern "C"
{
#endif

/// Call this from an application context, when there is data to send.
void commsCcfTxAvailable();

void commsCcfUartIsr();

#ifdef __cplusplus
};
#endif
