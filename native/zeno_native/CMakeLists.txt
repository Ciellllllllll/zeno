cmake_minimum_required(VERSION 3.24)

project(zeno_native LANGUAGES CXX)

add_library(zeno_native INTERFACE)
target_compile_features(zeno_native INTERFACE cxx_std_20)
target_include_directories(zeno_native INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

set(ZENO_ABI_TARGET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../target/debug")

add_library(zeno_abi_rust SHARED IMPORTED GLOBAL)
set_target_properties(zeno_abi_rust PROPERTIES
    IMPORTED_IMPLIB "${ZENO_ABI_TARGET_DIR}/zeno_abi.dll.lib"
    IMPORTED_LOCATION "${ZENO_ABI_TARGET_DIR}/zeno_abi.dll"
)

add_executable(zeno_abi_smoke src/abi_smoke.cpp)
target_compile_features(zeno_abi_smoke PRIVATE cxx_std_20)
target_include_directories(zeno_abi_smoke PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(zeno_abi_smoke PRIVATE zeno_abi_rust)

add_custom_command(TARGET zeno_abi_smoke POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_abi_smoke>"
)
