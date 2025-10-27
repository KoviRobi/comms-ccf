#include "rpc-task.h"

/// Demo headers
#include "ccf-config.hpp"
#include "ccf-uart0.h"

/// Comms-CCF headers
#include <ccf.hpp>
#include <rpc.hpp>

#include <FreeRTOS.h>
#include <task.h>

#include <stdint.h>

#include <array>
#include <atomic>
#include <string_view>

using namespace std::literals;

static StackType_t commsCcfRpcStack[512];
static StaticTask_t commsCcfRpcTcb;

TaskHandle_t rpcTask;

/// Add RPC definitions here.
static const Rpc rpc{
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

/// The RPC handler task loop.
void commsCcfRpcTask(void *)
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
void commsCcfStartRpcTask(void)
{
#if !defined(CCF_FEATURES) || CCF_FEATURES > 0
    rpcTask = xTaskCreateStatic(
        commsCcfRpcTask,
        "Rpc",
        sizeof(commsCcfRpcStack)/sizeof(StackType_t),
        NULL, // Task args
        tskIDLE_PRIORITY, // Priority
        commsCcfRpcStack,
        &commsCcfRpcTcb);
#endif
}
