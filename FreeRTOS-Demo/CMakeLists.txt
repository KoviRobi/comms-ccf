cmake_minimum_required(VERSION 3.21)

project(FreeRTOS-Demo LANGUAGES C ASM)

set(FREERTOS_KERNEL "${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel")
set(FREERTOS_PORT GCC_ARM_CM3)
set(FREERTOS_HEAP 4)
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config INTERFACE .)
add_subdirectory("${FREERTOS_KERNEL}" FreeRTOS_Kernel)

add_executable(FreeRTOS-Demo
    startup.c
    main.c
)
target_include_directories(FreeRTOS-Demo PUBLIC
    .
    LocalDemoFiles
    LuminaryMicro
)
target_link_libraries(FreeRTOS-Demo PUBLIC
    freertos_kernel
    "${CMAKE_CURRENT_LIST_DIR}/LuminaryMicro/arm-none-eabi-gcc/libdriver.a"
    "${CMAKE_CURRENT_LIST_DIR}/LuminaryMicro/arm-none-eabi-gcc/libgr.a"
)

add_custom_target(run
    COMMAND qemu-system-arm
        -machine lm3s6965evb
        -monitor none
        -serial stdio
        -nographic
        -s # gdb tcp::1234
        -S
        -kernel $<TARGET_FILE:FreeRTOS-Demo>
    VERBATIM COMMAND_EXPAND_LISTS
    COMMENT "You can now attach GDB to localhost:1234"
    USES_TERMINAL
)
