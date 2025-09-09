# FreeRTOS Demo of Comms-CCF

This is using the Demo/CORTEX_LM3S6965_GCC_QEMU of my fork, with the
Demo/Common files removed, and the Demo/drivers/LuminaryMicro copied
(to avoid cloning the 1.5GiB FreeRTOS repo.

Also cleaned up the whitespace, and converted to LF line endings.

Adding comms-ccf is done in the commit
```
git show '@^{/FreeRTOS-Demo: Add RPC example}'
```
which is hopefully small and stand-alone enough that it is easy to
understand.

To run the demo, first do
```
cmake --workflow --preset=run-debug
```
and then you can run
```
python ../python/tcp.py
```

## Resource use

| commit                                                           | SRAM    | %age   | FLASH   | %age  |
|------------------------------------------------------------------|---------|--------|---------|-------|
| `@^{/FreeRTOS-Demo: Use stdio.h}`                                | 53872 B | 82.20% |20940 B  | 7.99% |
| `@^{/FreeRTOS-Demo: Add RPC example}`                            | 59348 B | 90.56% |25028 B  | 9.55% |
| `@^{/FreeRTOS-Demo: Add trivial second function}`                | 59376 B | 90.60% | 25092 B | 9.57% |
| `@^{/FreeRTOS-Demo: Add a version command to diff resource use}` | 59428 B | 90.68% | 25568 B | 9.75% |
