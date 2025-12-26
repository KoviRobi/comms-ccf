/**
\file
\brief C interface to use Comms-CCF over the LM3S6965 UART.
*/
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
