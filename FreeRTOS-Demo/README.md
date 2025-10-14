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
uv run comms-ccf-tcp
```

## Resource use

| Sumary                                                                            | SRAM  | FLASH  |
|-----------------------------------------------------------------------------------|-------|--------|
| 0. No Comms-CCF                                                                   | 3972B | 11416B |
| 1. Add Comms-CCF, no RPC handlers ([2KiB stack][1bss], [2KiB flash][1text])       | 6112B | 13524B |
| 2. Add version RPC call ([24B RAM][2bss], [904B flash][2text])                    | 6136B | 14428B |
| 3. Add `add` call ([28B RAM][3bss], [736BB flash][3text])                         | 6164B | 15164B |
| 4. Add `sub` call, similar to previous ([28B RAM][4bss], [60B flash][4text])      | 6192B | 15224B |
| 5. Add `greet` call ([24B RAM][5bss], [308B flash][5text])                        | 6216B | 15532B |
| 6  Add `read_mem` and `write_mem` calls ([56B RAM][6bss], [1.4KiBB flash][6text]) | 6272B | 16956B |
| 7. Add log task ([1KiB stack][7bss], [4KiB flash][7text])                         | 7384B | 21148B |

These were calculated with `python misc/measure_features.py`. The stack
sizes were calculated by looking at actual stack usage, and picking
a number about 2x that, to be safe. The flash usage could be lower,
but it's not terrible.

If you want more details of exactly what is added, look at either the
SVGs lined, or [misc/compare.md](misc/compare.md). The descriptions in
the table above link to an interactive SVG (open the "raw" SVG in your
browser, then you can click on areas/sections to zoom).

[1text]: https://kovirobi.github.io/comms-ccf/compare.0-1.svg#area-FLASH-00000000-output-.text-00000000
[1bss]: https://kovirobi.github.io/comms-ccf/compare.0-1.svg#area-SRAM-20000000-output-.bss-20000164
[2text]: https://kovirobi.github.io/comms-ccf/compare.1-2.svg#area-FLASH-00000000-output-.text-00000000
[2bss]: https://kovirobi.github.io/comms-ccf/compare.1-2.svg#area-SRAM-20000000-output-.bss-20000164
[3text]: https://kovirobi.github.io/comms-ccf/compare.2-3.svg#area-FLASH-00000000-output-.text-00000000
[3bss]: https://kovirobi.github.io/comms-ccf/compare.2-3.svg#area-SRAM-20000000-output-.bss-20000164
[4text]: https://kovirobi.github.io/comms-ccf/compare.3-4.svg#area-FLASH-00000000-output-.text-00000000
[4bss]: https://kovirobi.github.io/comms-ccf/compare.3-4.svg#area-SRAM-20000000-output-.bss-20000164
[5text]: https://kovirobi.github.io/comms-ccf/compare.4-5.svg#area-FLASH-00000000-output-.text-00000000
[5bss]: https://kovirobi.github.io/comms-ccf/compare.4-5.svg#area-SRAM-20000000-output-.bss-20000164
[6text]: https://kovirobi.github.io/comms-ccf/compare.5-6.svg#area-FLASH-00000000-output-.text-00000000
[6bss]: https://kovirobi.github.io/comms-ccf/compare.5-6.svg#area-SRAM-20000000-output-.bss-20000164
[7text]: https://kovirobi.github.io/comms-ccf/compare.6-7.svg#area-FLASH-00000000-output-.text-00000000
[7bss]: https://kovirobi.github.io/comms-ccf/compare.6-7.svg#area-SRAM-20000000-output-.bss-20000164
