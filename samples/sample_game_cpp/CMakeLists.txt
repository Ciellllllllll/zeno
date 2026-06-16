cmake_minimum_required(VERSION 3.24)

project(zeno_sample_game_cpp LANGUAGES CXX)

set(ZENO_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(ZENO_SDK_DIR "${ZENO_REPO_ROOT}/sdk/cpp")
set(ZENO_SAMPLE_ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

if(NOT TARGET zeno_sdk_cpp)
    add_subdirectory("${ZENO_SDK_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/zeno_sdk_cpp")
endif()

add_executable(zeno_sample_game_cpp
    src/main.cpp
    src/sample_module.cpp
)
target_compile_features(zeno_sample_game_cpp PRIVATE cxx_std_20)
target_link_libraries(zeno_sample_game_cpp PRIVATE zeno_sdk_cpp)

add_custom_command(TARGET zeno_sample_game_cpp POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_sample_game_cpp>"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${ZENO_SAMPLE_ASSET_DIR}"
        "$<TARGET_FILE_DIR:zeno_sample_game_cpp>/assets"
)

install(TARGETS zeno_sample_game_cpp RUNTIME DESTINATION bin)
install(DIRECTORY "${ZENO_SAMPLE_ASSET_DIR}/" DESTINATION bin/assets)
