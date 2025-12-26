#include "ccf-uart0.h"

/// Demo headers
#include "ccf-config.hpp"
#include "rpc-task.h"

/// Comms-CCF headers
#include <ccf.hpp>

/// LuminaryMicro includes
#include "hw_types.h"
#include "hw_memmap.h"
#include "uart.h"

#include <FreeRTOS.h>
#include <task.h>

#include <stdint.h>

#include <atomic>

/// Used to ensure we don't try to send a character while another one
/// is already in progress. When this is set, we are either about to
/// send a character, or the callback hasn't arrived yet.
static std::atomic_bool txBusy;

/// Called by the UART interrupt handler when it receives a character.
static void commsCcfRx(uint8_t byte)
{
    // Safety: called from the UART interrupt handler
    if (ccf.unsafeGetUnderlying().receiveCharacter(byte))
    {
        /// Uses FreeRTOS direct task notifications to act as a semaphore:
        /// this is called from an interrupt to tell the RPC handler
        /// task when there is something to process.
        if (rpcTask)
        {
            BaseType_t xHigherPriorityTaskWoken = pdFALSE;
            vTaskNotifyGiveFromISR(rpcTask, &xHigherPriorityTaskWoken);
            portYIELD_FROM_ISR(xHigherPriorityTaskWoken);
        }
    }
}

/// Try and send the next character
static void commsCcfTxNext()
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
        UARTCharPutNonBlocking(UART0_BASE, c);
    }
    // Not else -- iterator changed
    if (toTx && !(toTx->begin() != toTx->end()))
    {
        toTx.reset();
    }
}

/// Called by the UART interrupt handler when a character has finished
/// transmitting.
static void commsCcfTxDone()
{
    txBusy = false;
    commsCcfTxNext();
}

/// Note: because `commsCcfTxNext` calls `UARTCharPutNonBlocking` which
/// might finish instantly, we need to protect it with a critical section
/// to avoid issues due to reentrancy.
void commsCcfTxAvailable()
{
    /// \todo Check UART is not busy?
    portENTER_CRITICAL();
    if (!txBusy)
    {
        commsCcfTxNext();
    }
    portEXIT_CRITICAL();
}

void commsCcfUartIsr()
{
    const unsigned long interrupt = UARTIntStatus(UART0_BASE, pdTRUE);
    UARTIntClear(UART0_BASE, interrupt);
    if (interrupt & UART_INT_RX)
    {
        while (UARTCharsAvail(UART0_BASE))
        {
            const uint8_t byte = UARTCharGetNonBlocking(UART0_BASE);
            commsCcfRx( byte );
        }
    }
    if (interrupt & UART_INT_TX)
    {
        commsCcfTxDone();
    }
}
