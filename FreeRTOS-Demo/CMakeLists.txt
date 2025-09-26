cmake_minimum_required(VERSION 3.21)

set(CMAKE_CXX_STANDARD 23)

project(FreeRTOS-Demo LANGUAGES CXX C ASM)

set(FREERTOS_KERNEL "${CMAKE_CURRENT_LIST_DIR}/FreeRTOS-Kernel")
set(FREERTOS_PORT GCC_ARM_CM3)
add_library(freertos_config INTERFACE)
target_include_directories(freertos_config INTERFACE .)
add_subdirectory("${FREERTOS_KERNEL}" FreeRTOS_Kernel)

add_executable(FreeRTOS-Demo
    startup.c
    main.c
    log.cpp
    comms-ccf.cpp

    ../comms-ccf/cbor.cpp
    ../comms-ccf/cobs.cpp
)
target_include_directories(FreeRTOS-Demo PUBLIC
    .
    ../comms-ccf
    ../test
    LocalDemoFiles
    LuminaryMicro
)
target_link_libraries(FreeRTOS-Demo PUBLIC
    freertos_kernel
    "${CMAKE_CURRENT_LIST_DIR}/LuminaryMicro/arm-none-eabi-gcc/libdriver.a"
    "${CMAKE_CURRENT_LIST_DIR}/LuminaryMicro/arm-none-eabi-gcc/libgr.a"
)
cmake_path(SET debug_dir NORMALIZE "${CMAKE_CURRENT_LIST_DIR}/../debug")
target_compile_definitions(FreeRTOS-Demo PUBLIC
    "$<$<CONFIG:Debug>:DEBUG_CBOR=\"${debug_dir}/stderr.hpp\">"
    "$<$<CONFIG:Debug>:DEBUG_CCF=\"${debug_dir}/stderr.hpp\">"
    "$<$<CONFIG:Debug>:DEBUG_CIRC_BUF=\"${debug_dir}/stderr.hpp\">"
    "$<$<CONFIG:Debug>:DEBUG_COBS=\"${debug_dir}/stderr.hpp\">"
    "$<$<CONFIG:Debug>:DEBUG_FNV1A=\"${debug_dir}/stderr.hpp\">"
    "$<$<CONFIG:Debug>:DEBUG_RPC=\"${debug_dir}/stderr.hpp\">"
    "$<$<CONFIG:Debug>:STRIP_ROOT=\"${debug_dir}\">"
)

add_custom_target(run
    COMMAND qemu-system-arm
        -machine lm3s6965evb
        -monitor none
        -nographic
        -serial tcp::4321,server=on,nowait # UART0
        -serial stdio                      # UART1
        -s                                 # gdb tcp::1234
        # -S # waits for a deubgger, not needed most of the time
        -kernel $<TARGET_FILE:FreeRTOS-Demo>
    VERBATIM COMMAND_EXPAND_LISTS
    COMMENT "You can now attach GDB to localhost:1234"
    USES_TERMINAL
)

# For comparing sizes
foreach(CCF_FEATURES RANGE 7)
    set(TARGET "FreeRTOS-Demo-${CCF_FEATURES}")
    add_executable("${TARGET}" EXCLUDE_FROM_ALL
        "$<TARGET_PROPERTY:FreeRTOS-Demo,SOURCES>"
    )
     add_custom_command(TARGET "${TARGET}"
        POST_BUILD
        COMMAND python
            "${CMAKE_CURRENT_LIST_DIR}/../python/binutils_mapfile"
            "$<TARGET_FILE:${TARGET}>.map"
            --yaml "$<TARGET_FILE:${TARGET}>.map.yaml"
        COMMAND python
            "${CMAKE_CURRENT_LIST_DIR}/../python/binutils_mapfile"
            "$<TARGET_FILE:${TARGET}>.map"
            --svg "$<TARGET_FILE:${TARGET}>.map.svg"
        VERBATIM
        COMMAND_EXPAND_LISTS
    )
    set_target_properties("${TARGET}" PROPERTIES
        INCLUDE_DIRECTORIES "$<TARGET_PROPERTY:FreeRTOS-Demo,INCLUDE_DIRECTORIES>"
        LINK_LIBRARIES "$<TARGET_PROPERTY:FreeRTOS-Demo,LINK_LIBRARIES>"
    )
    target_compile_definitions("${TARGET}" PRIVATE
        "CCF_FEATURES=${CCF_FEATURES}")
    list(APPEND DEMO_MEASURE "${TARGET}")
endforeach()
add_custom_target(FreeRTOS-Demo-measure DEPENDS ${DEMO_MEASURE})
