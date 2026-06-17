cmake_minimum_required(VERSION 3.24)

project(zeno LANGUAGES CXX)

include(CTest)

option(ZENO_ENABLE_CLANG_TIDY "Run clang-tidy while compiling C++ targets." OFF)

if(ZENO_ENABLE_CLANG_TIDY)
    find_program(ZENO_CLANG_TIDY_EXE NAMES clang-tidy)
    if(NOT ZENO_CLANG_TIDY_EXE)
        message(FATAL_ERROR "ZENO_ENABLE_CLANG_TIDY is ON, but clang-tidy was not found on PATH.")
    endif()
    set(CMAKE_CXX_CLANG_TIDY "${ZENO_CLANG_TIDY_EXE}")
endif()

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_subdirectory(native)
add_subdirectory(sdk/cpp)
add_subdirectory(samples/sample_game_cpp)
add_subdirectory(samples/dynamic_module_cpp)
add_subdirectory(samples/sdk_feature_samples_cpp)
add_subdirectory(templates/game-cpp)

if(BUILD_TESTING)
    add_subdirectory(tests)
endif()

find_program(ZENO_POWERSHELL powershell)
if(ZENO_POWERSHELL)
    add_custom_target(zeno_package_sdk
        COMMAND "${ZENO_POWERSHELL}" -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/scripts/package-sdk.ps1" -Configuration "$<CONFIG>"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Packaging ZENO SDK for $<CONFIG>"
    )
endif()
