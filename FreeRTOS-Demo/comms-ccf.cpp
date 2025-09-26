#include "comms-ccf.hpp"

#include "ccf.hpp"
#include "rpc.hpp"

#include "FreeRTOS.h"
#include "task.h"

#include <stdint.h>

#include <array>
#include <atomic>
#include <string_view>

using namespace std::literals;

static StackType_t commsCcfStack[512];
static StaticTask_t commsCcfTcb;

Mutex<Ccf<{
    .rxBufSize = 256,
    .txBufSize = 256,
    .maxPktSize = 255,
}>> ccf;

static TaskHandle_t rxTask;

/// Used to ensure we don't try to send a character while another one
/// is already in progress. When this is set, we are either about to
/// send a character, or the callback hasn't arrived yet.
static std::atomic_bool txBusy;

/// Add RPC definitions here.
static Rpc rpc{
#if !defined(CCF_FEATURES) || CCF_FEATURES > 1
    /// Note: using `+` to convert lambda to a function pointer
    Call("version", "software version", {}, +[]() {
        return std::tuple{
            "main"sv,
            "a/b/c"sv,
            /*git hash*/0x1234567llu
        };
    }),
#endif // CCF_FEATURES > 1

#if !defined(CCF_FEATURES) || CCF_FEATURES > 2
    Call("add", "return x+y", {"x", "y"}, +[](uint32_t x, uint32_t y) { return x + y; }),
#endif // CCF_FEATURES > 2

#if !defined(CCF_FEATURES) || CCF_FEATURES > 3
    Call("sub", "return x-y", {"x", "y"}, +[](uint32_t x, uint32_t y) { return x - y; }),
#endif // CCF_FEATURES > 3

#if !defined(CCF_FEATURES) || CCF_FEATURES > 4
    Call("hello", "greet", {}, +[]() { return "Hello world"sv; }),
#endif // CCF_FEATURES > 4

#if !defined(CCF_FEATURES) || CCF_FEATURES > 5
    Call("read_mem", "read memory", {"addr", "size"}, +[](uintptr_t addr, size_t size) {
        return std::span(reinterpret_cast<uint8_t *>(addr), size);
    }),
    Call("write_mem", "write memory", {"addr", "data"},
        +[](uintptr_t addr, std::span<uint8_t> data) {
            std::ranges::copy(data, reinterpret_cast<uint8_t *>(addr));
        }
    ),
#endif // CCF_FEATURES > 5
};

/// Called by the UART interrupt handler when it receives a character.
void commsCcfRx(uint8_t byte)
{
    // Safety: called from the UART interrupt handler
    if (ccf.unsafeGetUnderlying().receiveCharacter(byte))
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

/// Try and send the next character
void commsCcfTxNext()
{
    static std::optional<decltype(ccf)::Underlying::TxFrame> toTx{};

    if (!toTx)
    {
        ccf.unsafeGetUnderlying().charactersToSend(toTx);
    }
    // Not else -- toTx may have changed
    if (toTx && toTx->begin() != toTx->end())
    {
        const char c = *toTx->begin();
        ++toTx->begin();
        txBusy = true;
        commsCcfTx(c);
    }
    // Not else -- iterator changed
    if (toTx && !(toTx->begin() != toTx->end()))
    {
        toTx.reset();
    }
}

/// Called by the UART interrupt handler when a character has finished
/// transmitting.
void commsCcfTxDone()
{
    txBusy = false;
    commsCcfTxNext();
}

/// Note: because `commsCcfTxNext` calls `commsCcfTx` which might finish
/// instantly, we need to protect it with a critical section to avoid
/// issues due to reentrancy.
void commsCcfTxAvailable()
{
    // TODO: Check UART is not busy?
    portENTER_CRITICAL();
    if (!txBusy)
    {
        commsCcfTxNext();
    }
    portEXIT_CRITICAL();
}

/// The RPC handler task loop.
void commsCcfProcessTask(void *)
{
    while (1)
    {
        // Safety: Called from a single thread
        // Poll first in case we skipped a notification on init
        if (ccf.unsafeGetUnderlying().poll(rpc))
        {
            // Kick TX
            commsCcfTxAvailable();
        }
        // Waiting for an RPC call to arrive -- woken by commsCcfNotify
        // above.
        TickType_t timeout = pdMS_TO_TICKS(1000);
        if (ulTaskNotifyTake(pdTRUE, timeout) == 0)
        {
            // Timeout
            // Maybe do some action e.g. ticke watchdog
            continue;
        }
    }
}

/// Start the task responsible for receiving and responding to RPC calls.
void commsCcfStartTasks(void)
{
#if !defined(CCF_FEATURES) || CCF_FEATURES > 0
    rxTask = xTaskCreateStatic(
        commsCcfProcessTask,
        "CommsCcf",
        sizeof(commsCcfStack)/sizeof(StackType_t),
        NULL, // Task args
        tskIDLE_PRIORITY, // Priority
        commsCcfStack,
        &commsCcfTcb);
#endif
}
