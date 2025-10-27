#pragma once

#include <FreeRTOS.h>
#include <task.h>

#ifdef __cplusplus
extern "C"
{
#endif

extern TaskHandle_t rpcTask;

/// Use this to start the RPC task (only one task handles RPCs because
/// it needs to wait for messages)
void commsCcfStartRpcTask(void);

#ifdef __cplusplus
};
#endif
