cmake_minimum_required(VERSION 3.15)

# Set some sensible defaults
set(CMAKE_EXPORT_COMPILE_COMMANDS ON
    CACHE BOOL "Enable/Disable output of compile commands during generation.")

set(CMAKE_BUILD_TYPE Debug
    CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel ...")

set(CMAKE_CXX_STANDARD 23)
# Note, if CMAKE_CXX_STANDARD is less than 23 then
# string(APPEND CMAKE_CXX_FLAGS " -DNO_FLOAT16")

project(comms-ccf CXX)

# WIP TODO
add_executable(itertools comms-ccf/itertools.cpp)

enable_testing()

set(test_build_deps "")

# Helper for building test dependencies and running the test
function(add_build_and_test)
    set(options)
    set(oneValueArgs NAME)
    set(multiValueArgs DEPENDS COMMAND)
    cmake_parse_arguments(PARSE_ARGV 0 arg
        "${options}" "${oneValueArgs}" "${multiValueArgs}"
    )

    add_test(NAME "${arg_NAME}" COMMAND ${arg_COMMAND})
    set(test_build_deps ${test_build_deps} ${arg_DEPENDS} PARENT_SCOPE)
    set_tests_properties("${arg_NAME}" PROPERTIES FIXTURES_REQUIRED f_build)
endfunction()

# A demo for inspecting the encode and decode of COBS
add_executable(cobs test/cobs.cpp comms-ccf/cobs.cpp)
target_include_directories(cobs PUBLIC comms-ccf/ test/)
add_build_and_test(NAME cobs_demo DEPENDS cobs COMMAND $<TARGET_FILE:cobs>)

# A couple of tests for the circular buffer
add_executable(circular_buffer test/circular_buffer.cpp)
target_include_directories(circular_buffer PUBLIC comms-ccf/ test/)
add_build_and_test(
    NAME circular_buffer_test
    DEPENDS circular_buffer
    COMMAND $<TARGET_FILE:circular_buffer>
)

# Tests the CBOR encoding based on the table in comms-ccf/cbor.hpp
add_executable(cbor test/cbor.cpp comms-ccf/cbor.cpp)
target_include_directories(cbor PUBLIC comms-ccf/ test/)
add_build_and_test(
    NAME cbor_test
    DEPENDS cbor
    COMMAND $<TARGET_FILE:cbor> "${CMAKE_CURRENT_LIST_DIR}/comms-ccf/cbor.hpp"
)

# Demo of RPC calls by encoding/decoding in Python
add_executable(rpc test/rpc.cpp comms-ccf/cobs.cpp comms-ccf/cbor.cpp)
target_include_directories(rpc PUBLIC comms-ccf/)
add_build_and_test(
    NAME rpc_demo
    DEPENDS rpc
    COMMAND
        uv run comms-ccf
        --script-file "${CMAKE_CURRENT_LIST_DIR}/test/rpc.interactive"
        --verbose --no-repl -- stdio $<TARGET_FILE:rpc>
)

add_executable(iter test/iter.cpp comms-ccf/cobs.cpp comms-ccf/cbor.cpp)
target_include_directories(iter PUBLIC comms-ccf/)
add_build_and_test(
    NAME iter_demo
    DEPENDS iter
    COMMAND $<TARGET_FILE:iter>
)

add_executable(rpc_inline_vtable test/rpc.cpp comms-ccf/cobs.cpp comms-ccf/cbor.cpp)
target_compile_definitions(rpc_inline_vtable PUBLIC INLINE_VTABLE)
target_include_directories(rpc_inline_vtable PUBLIC comms-ccf/)
add_build_and_test(
    NAME rpc_inline_vtable_demo
    DEPENDS rpc_inline_vtable
    COMMAND
        uv run comms-ccf
        --script-file "${CMAKE_CURRENT_LIST_DIR}/test/rpc.interactive"
        --verbose --no-repl -- stdio $<TARGET_FILE:rpc>
)

add_executable(rpc_debug test/rpc.cpp comms-ccf/cobs.cpp comms-ccf/cbor.cpp)
target_include_directories(rpc_debug PUBLIC comms-ccf/)
target_compile_definitions(rpc_debug PUBLIC
    "DEBUG_CBOR=\"${CMAKE_CURRENT_LIST_DIR}/debug/stderr.hpp\""
    "DEBUG_CCF=\"${CMAKE_CURRENT_LIST_DIR}/debug/stderr.hpp\""
    "DEBUG_COBS=\"${CMAKE_CURRENT_LIST_DIR}/debug/stderr.hpp\""
    "DEBUG_FNV1A=\"${CMAKE_CURRENT_LIST_DIR}/debug/stderr.hpp\""
    "DEBUG_RPC=\"${CMAKE_CURRENT_LIST_DIR}/debug/stderr.hpp\""
    "STRIP_ROOT=\"${CMAKE_CURRENT_LIST_DIR}/\""
)
add_build_and_test(
    NAME rpc_debug_demo
    DEPENDS rpc_debug
    COMMAND
        uv run comms-ccf
        --script-file "${CMAKE_CURRENT_LIST_DIR}/test/rpc.interactive"
        --verbose --no-repl -- stdio $<TARGET_FILE:rpc_debug>
)

# Demo of logging
add_executable(log test/log.cpp comms-ccf/cobs.cpp comms-ccf/cbor.cpp)
target_include_directories(log PUBLIC comms-ccf/)
add_build_and_test(
    NAME log_demo
    DEPENDS log
    COMMAND
        uv run comms-ccf --no-repl
        --expect-logs "${CMAKE_CURRENT_LIST_DIR}/test/log.expected"
        -- stdio $<TARGET_FILE:log>
)
add_executable(log_deferred test/log.cpp comms-ccf/cobs.cpp comms-ccf/cbor.cpp)
target_compile_definitions(log_deferred PUBLIC DEFERRED_FORMATTING)
target_include_directories(log_deferred PUBLIC comms-ccf/)
add_build_and_test(
    NAME log_deferred_demo
    DEPENDS log_deferred
    COMMAND
        uv run comms-ccf --verbose --no-repl
        --expect-logs "${CMAKE_CURRENT_LIST_DIR}/test/log.expected"
        -- stdio $<TARGET_FILE:log>
)

add_library(cobs_wrapper SHARED test/cobs_wrapper.cpp comms-ccf/cobs.cpp)
target_include_directories(cobs_wrapper PUBLIC comms-ccf/ test/)
add_library(fnv1a_wrapper SHARED test/fnv1a_wrapper.cpp)
target_include_directories(fnv1a_wrapper PUBLIC comms-ccf/ test/)
add_build_and_test(
    NAME python_cosimulate_test
    DEPENDS cobs_wrapper fnv1a_wrapper
    COMMAND pytest "${CMAKE_CURRENT_LIST_DIR}/test"
        --libcobs $<TARGET_FILE:cobs_wrapper>
        --libfnv1a $<TARGET_FILE:fnv1a_wrapper>
)

foreach(test IN LISTS test_build_deps)
    target_compile_options(${test} PUBLIC
        -Wall
        -Wextra
        -Wshadow
        -Wmissing-declarations
        $<$<CONFIG:Debug>:-g3 -Og>
        $<$<COMPILE_LANGUAGE:C>:-Wmissing-prototypes>
        $<$<COMPILE_LANGUAGE:CXX>:-fconcepts-diagnostics-depth=2>
    )
endforeach()

add_custom_target(build_test_deps DEPENDS ${test_build_deps})
add_test(build_test_deps
  "${CMAKE_COMMAND}"
  --build "${CMAKE_BINARY_DIR}"
  --config "$<CONFIG>"
  --target build_test_deps
)
set_tests_properties(build_test_deps PROPERTIES FIXTURES_SETUP f_build)
