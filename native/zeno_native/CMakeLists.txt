cmake_minimum_required(VERSION 3.24)

project(zeno_native LANGUAGES CXX)

set(ZENO_RUST_PROFILE "debug" CACHE STRING "Cargo profile directory that contains zeno_abi artifacts.")

add_library(zeno_native STATIC
    src/native_backend.cpp
    src/native_backend_c_api.cpp
)
target_compile_features(zeno_native PUBLIC cxx_std_20)
target_include_directories(zeno_native PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(zeno_native PRIVATE user32 d3d11 dxgi d3dcompiler)

set(ZENO_ABI_TARGET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../../target/${ZENO_RUST_PROFILE}")

if(NOT TARGET zeno_abi_rust)
    add_library(zeno_abi_rust SHARED IMPORTED GLOBAL)
    set_target_properties(zeno_abi_rust PROPERTIES
        IMPORTED_IMPLIB "${ZENO_ABI_TARGET_DIR}/zeno_abi.dll.lib"
        IMPORTED_LOCATION "${ZENO_ABI_TARGET_DIR}/zeno_abi.dll"
    )
endif()

add_executable(zeno_abi_smoke src/abi_smoke.cpp)
target_compile_features(zeno_abi_smoke PRIVATE cxx_std_20)
target_include_directories(zeno_abi_smoke PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(zeno_abi_smoke PRIVATE zeno_abi_rust)
add_test(NAME zeno_abi_smoke COMMAND zeno_abi_smoke)

add_custom_command(TARGET zeno_abi_smoke POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_abi_smoke>"
)

add_executable(zeno_native_backend_smoke src/native_backend_smoke.cpp)
target_compile_features(zeno_native_backend_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_native_backend_smoke PRIVATE zeno_native)
add_test(NAME zeno_native_backend_smoke COMMAND zeno_native_backend_smoke)
