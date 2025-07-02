cmake_minimum_required(VERSION 3.15)

# Set some sensible defaults
set(CMAKE_EXPORT_COMPILE_COMMANDS ON
    CACHE BOOL "Enable/Disable output of compile commands during generation.")

set(CMAKE_BUILD_TYPE Debug
    CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel ...")

set(CMAKE_CXX_STANDARD 20)

project(comms-ccf CXX)

string(APPEND CMAKE_CXX_FLAGS_DEBUG " -g3 -Og")
string(APPEND CMAKE_CXX_FLAGS " -fconcepts-diagnostics-depth=2")

add_executable(cobs comms-ccf/cobs.cpp test/cobs.cpp)
target_include_directories(cobs PUBLIC comms-ccf/ test/)

enable_testing()

set(components cobs)

set(test_args "")
foreach(component IN LISTS components)
    list(APPEND test_args "--lib${component}" $<TARGET_FILE:${component}_test>)
endforeach()

add_test(
    NAME python_cosimulate_test
    COMMAND pytest "${CMAKE_CURRENT_LIST_DIR}/test" ${test_args}
    WORKING_DIRECTORY "$ENV{PWD}"
)

foreach(component IN LISTS components)
    add_library(${component}_test SHARED test/${component}_wrapper.cpp)
    target_include_directories(${component}_test PUBLIC comms-ccf/ test/)
    target_compile_options(${component}_test PUBLIC -Wall -Wextra
        -Wmissing-declarations
        $<$<COMPILE_LANGUAGE:C>:-Wmissing-prototypes>
    )

    add_test(build_library_${component}_test
      "${CMAKE_COMMAND}"
      --build "${CMAKE_BINARY_DIR}"
      --config "$<CONFIG>"
      --target ${component}_test
    )

    set_tests_properties(build_library_${component}_test PROPERTIES FIXTURES_SETUP f_library_build)
endforeach()

target_sources(cobs_test PRIVATE comms-ccf/cobs.cpp)

set_tests_properties(python_cosimulate_test PROPERTIES  FIXTURES_REQUIRED f_library_build)
