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

| Sumary                                                                            | SRAM  | FLASH  |
|-----------------------------------------------------------------------------------|-------|--------|
| 0. No Comms-CCF                                                                   | 3972B | 11416B |
| 1. Add Comms-CCF, no RPC handlers ([2KiB stack][1bss], [2KiB flash][1text])       | 6112B | 13524B |
| 2. Add version RPC call ([24B RAM][1bss], [904B flash][1text])                    | 6136B | 14428B |
| 3. Add `add` call ([28B RAM][1bss], [736BB flash][1text])                         | 6164B | 15164B |
| 4. Add `sub` call, similar to previous ([28B RAM][1bss], [60B flash][1text])      | 6192B | 15224B |
| 5. Add `greet` call ([24B RAM][1bss], [308B flash][1text])                        | 6216B | 15532B |
| 6  Add `read_mem` and `write_mem` calls ([56B RAM][1bss], [1.4KiBB flash][1text]) | 6272B | 16956B |
| 7. Add log task ([1KiB stack][1bss], [4KiB flash][1text])                         | 7384B | 21148B |

These were calculated with `python misc/measure_features.py`. The stack
sizes were calculated by looking at actual stack usage, and picking
a number about 2x that, to be safe. The flash usage could be lower,
but it's not terrible.

If you want more details of exactly what is added, look at either the
SVGs lined, or [misc/compare.md](misc/compare.md). The descriptions in
the table above link to an interactive SVG (open the "raw" SVG in your
browser, then you can click on areas/sections to zoom).

[1text]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-FLASH-00000000-output-.text-00000000
[1bss]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-SRAM-20000000-output-.bss-20000164
[2text]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-FLASH-00000000-output-.text-00000000
[2bss]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-SRAM-20000000-output-.bss-20000164
[3text]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-FLASH-00000000-output-.text-00000000
[3bss]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-SRAM-20000000-output-.bss-20000164
[4text]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-FLASH-00000000-output-.text-00000000
[4bss]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-SRAM-20000000-output-.bss-20000164
[5text]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-FLASH-00000000-output-.text-00000000
[5bss]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-SRAM-20000000-output-.bss-20000164
[6text]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-FLASH-00000000-output-.text-00000000
[6bss]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-SRAM-20000000-output-.bss-20000164
[7text]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-FLASH-00000000-output-.text-00000000
[7bss]: https://github.com/KoviRobi/comms-ccf/raw/main/FreeRTOS-Demo/misc/compare-0-1.svg#area-SRAM-20000000-output-.bss-20000164
