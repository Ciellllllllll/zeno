cmake_minimum_required(VERSION 3.24)

project(zeno LANGUAGES CXX)

include(CTest)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_subdirectory(native)
add_subdirectory(sdk/cpp)
add_subdirectory(samples/sample_game_cpp)
add_subdirectory(samples/dynamic_module_cpp)
add_subdirectory(samples/sdk_feature_samples_cpp)
add_subdirectory(templates/game-cpp)

find_program(ZENO_POWERSHELL powershell)
if(ZENO_POWERSHELL)
    add_custom_target(zeno_package_sdk
        COMMAND "${ZENO_POWERSHELL}" -ExecutionPolicy Bypass -File "${CMAKE_SOURCE_DIR}/scripts/package-sdk.ps1" -Configuration "$<CONFIG>"
        WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
        COMMENT "Packaging ZENO SDK for $<CONFIG>"
    )
endif()
