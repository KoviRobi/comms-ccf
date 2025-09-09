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
