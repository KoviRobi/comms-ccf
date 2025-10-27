#include "log-task.h"

/// Demo headers
#include "ccf-config.hpp"
#include "ccf-uart0.h"

/// Comms-CCF headers
#include <ccf.hpp>

#include <FreeRTOS.h>
#include <task.h>

#include <stdio.h>
#include <string.h>

static StackType_t logStack[256];
static StaticTask_t logTcb;
static void logTask(void *)
{
    while (true)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        if (ccf.lock()->log(LogLevel::Info, 0, "Test log %d", 123))
        {
            commsCcfTxAvailable();
        }
    }
}

void commsCcfStartLogTask()
{
    xTaskCreateStatic(
        logTask,
        "Log",
        sizeof(logStack)/sizeof(StackType_t),
        NULL, // Task args
        tskIDLE_PRIORITY, // Priority
        logStack,
        &logTcb);
}
