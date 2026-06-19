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
target_link_libraries(zeno_native PRIVATE user32 d3d11 dxgi d3dcompiler windowscodecs ole32 xaudio2)

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
set_tests_properties(zeno_abi_smoke PROPERTIES LABELS "headless")

add_custom_command(TARGET zeno_abi_smoke POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_abi_smoke>"
)

add_executable(zeno_native_backend_smoke src/native_backend_smoke.cpp)
target_compile_features(zeno_native_backend_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_native_backend_smoke PRIVATE zeno_native)
add_test(NAME zeno_window_native_backend_smoke COMMAND zeno_native_backend_smoke)
set_tests_properties(zeno_window_native_backend_smoke PROPERTIES LABELS "window")

add_executable(zeno_renderer_backend_smoke src/renderer_backend_smoke.cpp)
target_compile_features(zeno_renderer_backend_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_renderer_backend_smoke PRIVATE zeno_native)
add_test(NAME zeno_renderer_backend_smoke COMMAND zeno_renderer_backend_smoke)
set_tests_properties(zeno_renderer_backend_smoke PROPERTIES LABELS "headless")

add_executable(zeno_resize_smoke src/resize_smoke.cpp)
target_compile_features(zeno_resize_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_resize_smoke PRIVATE zeno_native)
add_test(NAME zeno_window_resize_smoke COMMAND zeno_resize_smoke)
set_tests_properties(zeno_window_resize_smoke PROPERTIES LABELS "window")

add_executable(zeno_input_smoke src/input_smoke.cpp)
target_compile_features(zeno_input_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_input_smoke PRIVATE zeno_native)
add_test(NAME zeno_input_smoke COMMAND zeno_input_smoke)
set_tests_properties(zeno_input_smoke PROPERTIES LABELS "headless")

add_executable(zeno_shader_smoke src/shader_smoke.cpp)
target_compile_features(zeno_shader_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_shader_smoke PRIVATE zeno_native)
add_test(NAME zeno_shader_smoke COMMAND zeno_shader_smoke)
set_tests_properties(zeno_shader_smoke PROPERTIES LABELS "headless")

add_executable(zeno_texture_smoke src/texture_smoke.cpp)
target_compile_features(zeno_texture_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_texture_smoke PRIVATE zeno_native)
add_test(NAME zeno_texture_smoke COMMAND zeno_texture_smoke)
set_tests_properties(zeno_texture_smoke PROPERTIES LABELS "headless")

add_executable(zeno_mesh_smoke src/mesh_smoke.cpp)
target_compile_features(zeno_mesh_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_mesh_smoke PRIVATE zeno_native)
add_test(NAME zeno_mesh_smoke COMMAND zeno_mesh_smoke)
set_tests_properties(zeno_mesh_smoke PROPERTIES LABELS "headless")

add_executable(zeno_material_smoke src/material_smoke.cpp)
target_compile_features(zeno_material_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_material_smoke PRIVATE zeno_native)
add_test(NAME zeno_material_smoke COMMAND zeno_material_smoke)
set_tests_properties(zeno_material_smoke PROPERTIES LABELS "headless")

add_executable(zeno_audio_smoke src/audio_smoke.cpp)
target_compile_features(zeno_audio_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_audio_smoke PRIVATE zeno_native)
add_test(NAME zeno_audio_smoke COMMAND zeno_audio_smoke)
set_tests_properties(zeno_audio_smoke PROPERTIES LABELS "headless")

add_executable(zeno_debug_draw_smoke src/debug_draw_smoke.cpp)
target_compile_features(zeno_debug_draw_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_debug_draw_smoke PRIVATE zeno_native)
add_test(NAME zeno_debug_draw_smoke COMMAND zeno_debug_draw_smoke)
set_tests_properties(zeno_debug_draw_smoke PROPERTIES LABELS "headless")
