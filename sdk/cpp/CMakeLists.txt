cmake_minimum_required(VERSION 3.24)

project(zeno_sdk_cpp LANGUAGES CXX)

set(ZENO_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(ZENO_NATIVE_DIR "${ZENO_REPO_ROOT}/native/zeno_native")
set(ZENO_RUST_PROFILE "debug" CACHE STRING "Cargo profile directory that contains zeno_abi artifacts.")
set(ZENO_ABI_TARGET_DIR "${ZENO_REPO_ROOT}/target/${ZENO_RUST_PROFILE}")

if(NOT TARGET zeno_abi_rust)
    add_library(zeno_abi_rust SHARED IMPORTED GLOBAL)
    set_target_properties(zeno_abi_rust PROPERTIES
        IMPORTED_IMPLIB "${ZENO_ABI_TARGET_DIR}/zeno_abi.dll.lib"
        IMPORTED_LOCATION "${ZENO_ABI_TARGET_DIR}/zeno_abi.dll"
    )
endif()

if(NOT TARGET zeno_native)
    add_subdirectory("${ZENO_NATIVE_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/zeno_native")
endif()

add_library(zeno_sdk_cpp STATIC
    src/zeno.cpp
    src/asset.cpp
    src/math.cpp
    src/scene.cpp
    src/serialization.cpp
)
target_compile_features(zeno_sdk_cpp PUBLIC cxx_std_20)
target_include_directories(zeno_sdk_cpp PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${ZENO_NATIVE_DIR}/include
)
target_link_libraries(zeno_sdk_cpp PUBLIC zeno_abi_rust)
target_link_libraries(zeno_sdk_cpp PUBLIC zeno_native)

add_executable(zeno_sdk_smoke src/sdk_smoke.cpp)
target_compile_features(zeno_sdk_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_sdk_smoke PRIVATE zeno_sdk_cpp)
add_test(NAME zeno_window_sdk_smoke COMMAND zeno_sdk_smoke)
set_tests_properties(zeno_window_sdk_smoke PROPERTIES LABELS "window")

add_executable(zeno_sdk_math_smoke src/math_smoke.cpp)
target_compile_features(zeno_sdk_math_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_sdk_math_smoke PRIVATE zeno_sdk_cpp)
add_test(NAME zeno_sdk_math_smoke COMMAND zeno_sdk_math_smoke)
set_tests_properties(zeno_sdk_math_smoke PROPERTIES LABELS "headless")

add_executable(zeno_sdk_collision_smoke src/collision_smoke.cpp)
target_compile_features(zeno_sdk_collision_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_sdk_collision_smoke PRIVATE zeno_sdk_cpp)
add_test(NAME zeno_sdk_collision_smoke COMMAND zeno_sdk_collision_smoke)
set_tests_properties(zeno_sdk_collision_smoke PROPERTIES LABELS "headless")

add_executable(zeno_sdk_asset_smoke src/asset_smoke.cpp)
target_compile_features(zeno_sdk_asset_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_sdk_asset_smoke PRIVATE zeno_sdk_cpp)
add_test(NAME zeno_sdk_asset_smoke COMMAND zeno_sdk_asset_smoke)
set_tests_properties(zeno_sdk_asset_smoke PROPERTIES LABELS "headless")

add_executable(zeno_sdk_audio_smoke src/audio_smoke.cpp)
target_compile_features(zeno_sdk_audio_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_sdk_audio_smoke PRIVATE zeno_sdk_cpp)
add_test(NAME zeno_sdk_audio_smoke COMMAND zeno_sdk_audio_smoke)
set_tests_properties(zeno_sdk_audio_smoke PROPERTIES LABELS "headless")

add_executable(zeno_sdk_app_smoke src/app_smoke.cpp)
target_compile_features(zeno_sdk_app_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_sdk_app_smoke PRIVATE zeno_sdk_cpp)
add_test(NAME zeno_sdk_app_smoke COMMAND zeno_sdk_app_smoke)
set_tests_properties(zeno_sdk_app_smoke PROPERTIES LABELS "headless")

add_executable(zeno_sdk_failed_init_smoke src/app_failed_init_smoke.cpp)
target_compile_features(zeno_sdk_failed_init_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_sdk_failed_init_smoke PRIVATE zeno_sdk_cpp)
add_test(NAME zeno_window_sdk_failed_init_smoke COMMAND zeno_sdk_failed_init_smoke)
set_tests_properties(zeno_window_sdk_failed_init_smoke PROPERTIES LABELS "window")

add_executable(zeno_sdk_scene_smoke src/scene_smoke.cpp)
target_compile_features(zeno_sdk_scene_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_sdk_scene_smoke PRIVATE zeno_sdk_cpp)
add_test(NAME zeno_sdk_scene_smoke COMMAND zeno_sdk_scene_smoke)
set_tests_properties(zeno_sdk_scene_smoke PROPERTIES LABELS "headless")

add_executable(zeno_sdk_serialization_smoke src/serialization_smoke.cpp)
target_compile_features(zeno_sdk_serialization_smoke PRIVATE cxx_std_20)
target_link_libraries(zeno_sdk_serialization_smoke PRIVATE zeno_sdk_cpp)
add_test(NAME zeno_sdk_serialization_smoke COMMAND zeno_sdk_serialization_smoke)
set_tests_properties(zeno_sdk_serialization_smoke PROPERTIES LABELS "headless")

add_custom_command(TARGET zeno_sdk_smoke POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_sdk_smoke>"
)

add_custom_command(TARGET zeno_sdk_math_smoke POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_sdk_math_smoke>"
)

add_custom_command(TARGET zeno_sdk_collision_smoke POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_sdk_collision_smoke>"
)

add_custom_command(TARGET zeno_sdk_asset_smoke POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_sdk_asset_smoke>"
)

add_custom_command(TARGET zeno_sdk_audio_smoke POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_sdk_audio_smoke>"
)

add_custom_command(TARGET zeno_sdk_app_smoke POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_sdk_app_smoke>"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:zeno_sdk_app_smoke>/assets"
)

add_custom_command(TARGET zeno_sdk_failed_init_smoke POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_sdk_failed_init_smoke>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/testdata/app_failed_init"
        "$<TARGET_FILE_DIR:zeno_sdk_failed_init_smoke>/assets"
    COMMAND ${CMAKE_COMMAND} -E make_directory
        "$<TARGET_FILE_DIR:zeno_sdk_failed_init_smoke>/assets/audio"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ZENO_REPO_ROOT}/samples/sample_game_cpp/assets/audio/sample_click.wav"
        "$<TARGET_FILE_DIR:zeno_sdk_failed_init_smoke>/assets/audio/sample_click.wav"
)

add_custom_command(TARGET zeno_sdk_scene_smoke POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_sdk_scene_smoke>"
)

add_custom_command(TARGET zeno_sdk_serialization_smoke POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_sdk_serialization_smoke>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/testdata/scene_serialization"
        "$<TARGET_FILE_DIR:zeno_sdk_serialization_smoke>/scene_serialization"
)
