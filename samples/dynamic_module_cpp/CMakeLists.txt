cmake_minimum_required(VERSION 3.24)

project(zeno_dynamic_module_cpp LANGUAGES CXX)

set(ZENO_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(ZENO_SDK_DIR "${ZENO_REPO_ROOT}/sdk/cpp")

if(NOT TARGET zeno_sdk_cpp)
    add_subdirectory("${ZENO_SDK_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/zeno_sdk_cpp")
endif()

add_library(zeno_dynamic_sample_module MODULE
    src/dynamic_module.cpp
)
target_compile_features(zeno_dynamic_sample_module PRIVATE cxx_std_20)
target_link_libraries(zeno_dynamic_sample_module PRIVATE zeno_sdk_cpp)

add_library(zeno_dynamic_bad_version_module MODULE
    src/bad_version_module.cpp
)
target_compile_features(zeno_dynamic_bad_version_module PRIVATE cxx_std_20)
target_link_libraries(zeno_dynamic_bad_version_module PRIVATE zeno_sdk_cpp)

add_library(zeno_dynamic_missing_entry_module MODULE
    src/missing_entry_module.cpp
)
target_compile_features(zeno_dynamic_missing_entry_module PRIVATE cxx_std_20)

add_executable(zeno_dynamic_module_sample
    src/main.cpp
)
target_compile_features(zeno_dynamic_module_sample PRIVATE cxx_std_20)
target_link_libraries(zeno_dynamic_module_sample PRIVATE zeno_sdk_cpp)
add_dependencies(zeno_dynamic_module_sample
    zeno_dynamic_sample_module
    zeno_dynamic_bad_version_module
    zeno_dynamic_missing_entry_module
)

add_custom_command(TARGET zeno_dynamic_module_sample POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_dynamic_sample_module>"
        "$<TARGET_FILE_DIR:zeno_dynamic_module_sample>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_dynamic_bad_version_module>"
        "$<TARGET_FILE_DIR:zeno_dynamic_module_sample>"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_dynamic_missing_entry_module>"
        "$<TARGET_FILE_DIR:zeno_dynamic_module_sample>"
)

add_test(NAME zeno_dynamic_module_smoke COMMAND zeno_dynamic_module_sample)
set_tests_properties(zeno_dynamic_module_smoke PROPERTIES LABELS "headless")
