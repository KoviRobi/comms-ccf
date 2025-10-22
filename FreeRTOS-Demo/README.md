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
uv run comms-ccf tcp
```

## Resource use

<!-- Table starts here -->

| Summary                                | .text                         | .bss                        | .data             |
|----------------------------------------|-------------------------------|-----------------------------|-------------------|
| No Comms-CCF                           | 11.15KiB                      | 3.53KiB                     | 356B              |
| Add Comms-CCF, no RPC handlers         | [13.21KiB (+2.06Ki)][.text1]  | [5.62KiB (+2.09Ki)][.bss1]  | 356B (no change)  |
| Add version RPC call                   | [14.09KiB (+904)][.text2]     | [5.64KiB (+24)][.bss2]      | 356B (no change)  |
| Add `add` call                         | [14.81KiB (+736)][.text3]     | [5.67KiB (+28)][.bss3]      | 356B (no change)  |
| Add `sub` call, similar to `add`       | [14.87KiB (+60)][.text4]      | [5.70KiB (+28)][.bss4]      | 356B (no change)  |
| Add `greet` call                       | [15.17KiB (+308)][.text5]     | [5.72KiB (+24)][.bss5]      | 356B (no change)  |
| Add `readm_mem` and `write_mem` calls  | [16.56KiB (+1.39Ki)][.text6]  | [5.78KiB (+56)][.bss6]      | 356B (no change)  |
| Add test log task                      | [20.65KiB (+4.09Ki)][.text7]  | [6.86KiB (+1.09Ki)][.bss7]  | 356B (no change)  |

[.text1]: https://kovirobi.github.io/comms-ccf/compare.0-1.svg#.text
[.text2]: https://kovirobi.github.io/comms-ccf/compare.1-2.svg#.text
[.text3]: https://kovirobi.github.io/comms-ccf/compare.2-3.svg#.text
[.text4]: https://kovirobi.github.io/comms-ccf/compare.3-4.svg#.text
[.text5]: https://kovirobi.github.io/comms-ccf/compare.4-5.svg#.text
[.text6]: https://kovirobi.github.io/comms-ccf/compare.5-6.svg#.text
[.text7]: https://kovirobi.github.io/comms-ccf/compare.6-7.svg#.text
[.text8]: https://kovirobi.github.io/comms-ccf/compare.7-8.svg#.text
[.bss1]: https://kovirobi.github.io/comms-ccf/compare.0-1.svg#.bss
[.bss2]: https://kovirobi.github.io/comms-ccf/compare.1-2.svg#.bss
[.bss3]: https://kovirobi.github.io/comms-ccf/compare.2-3.svg#.bss
[.bss4]: https://kovirobi.github.io/comms-ccf/compare.3-4.svg#.bss
[.bss5]: https://kovirobi.github.io/comms-ccf/compare.4-5.svg#.bss
[.bss6]: https://kovirobi.github.io/comms-ccf/compare.5-6.svg#.bss
[.bss7]: https://kovirobi.github.io/comms-ccf/compare.6-7.svg#.bss
[.bss8]: https://kovirobi.github.io/comms-ccf/compare.7-8.svg#.bss

<!-- Table ends here -->

These were calculated with `python compare/update_table.py`. The stack
sizes were calculated by looking at actual stack usage, and picking
a number about 2x that, to be safe. The flash usage could be lower,
but it's not terrible.

If you want more details of exactly what is added, look at either the
SVGs linked, or [misc/compare.md][compare.md]. The descriptions in
the table above link to an interactive SVG (open the "raw" SVG in your
browser, then you can click on areas/sections to zoom).

[compare.md]: https://kovirobi.github.io/comms-ccf/compare.md
