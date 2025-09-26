#include "log.hpp"

#include "comms-ccf.hpp"

#include <ccf.hpp>

#include "hw_types.h"
#include "hw_memmap.h"
#include "uart.h"

#include <stdio.h>
#include <string.h>

#include <FreeRTOS.h>
#include <task.h>

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

void createLogTask()
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
