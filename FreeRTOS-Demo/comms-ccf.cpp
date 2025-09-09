#include "comms-ccf.hpp"

#include "ccf.hpp"
#include "rpc.hpp"

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>

using namespace std::literals;

static StackType_t commsCcfStack[1024];
static StaticTask_t commsCcfTcb;

static Ccf<{
    .rxBufSize = 256,
    .txBufSize = 256,
    .maxPktSize = 255,
}> ccf;

static TaskHandle_t rxTask;

/// Add RPC definitions here.
static Rpc rpc{
    /// Note: using `+` to convert lambda to a function pointer
    Call("add", "return x+y", {"x", "y"}, +[](uint32_t x, uint32_t y) { return x + y; }),
    Call("hello", "greet", {}, +[]() { return "Hello world"sv; }),
};


/// Called by the UART interrupt handler when it receives a character.
void commsCcfRx(uint8_t byte)
{
    if (ccf.receiveCharacter(byte))
    {
        /// Uses FreeRTOS direct task notifications to act as a semaphore:
        /// this is called from an interrupt to tell the RPC handler
        /// task when there is something to process.
        if (rxTask)
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(rxTask, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/// Called by the UART interrupt handler when a character has finished
/// transmitting.
void commsCcfTxNext()
{
    static std::optional<decltype(ccf)::TxNotification> toTx{};

    if (!toTx)
    {
        ccf.charactersToSend(toTx);
    }
    // Not else -- toTx may have changed
    if (toTx && toTx->begin() != toTx->end())
    {
        const char c = *toTx->begin();
        ++toTx->begin();
        commsCcfTx(c);
    }
    // Not else -- iterator changed
    if (toTx && !(toTx->begin() != toTx->end()))
    {
        toTx.reset();
    }
}

/// Note: because `commsCcfTxNext` calls `commsCcfTx` which might finish
/// instantly, we need to protect it with a critical section to avoid
/// issues due to reentrancy.
void commsCcfTxNextFromApplication()
{
    portENTER_CRITICAL();
    commsCcfTxNext();
    portEXIT_CRITICAL();
}

/// The RPC handler task loop.
void commsCcfProcessTask(void *)
{
    while (1)
    {
        // Waiting for an RPC call to arrive -- woken by commsCcfNotify
        // above.
        TickType_t timeout = pdMS_TO_TICKS(1000);
        if (ulTaskNotifyTake(pdTRUE, timeout) == 0)
        {
            // Timeout
            // Maybe do some action e.g. ticke watchdog
            continue;
        }
        ccf.poll(rpc);
        // Kick TX
        commsCcfTxNextFromApplication();
    }
}

/// Start the task responsible for receiving and responding to RPC calls.
void commsCcfStartTasks(void)
{
    rxTask = xTaskCreateStatic(
        commsCcfProcessTask,
        "CommsCcf",
        sizeof(commsCcfStack)/sizeof(StackType_t),
        NULL, // Task args
        tskIDLE_PRIORITY, // Priority
        commsCcfStack,
        &commsCcfTcb);
}
