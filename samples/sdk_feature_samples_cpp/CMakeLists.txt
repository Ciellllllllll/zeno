cmake_minimum_required(VERSION 3.24)

project(zeno_sdk_feature_samples_cpp LANGUAGES CXX)

set(ZENO_REPO_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
set(ZENO_SDK_DIR "${ZENO_REPO_ROOT}/sdk/cpp")
set(ZENO_SHARED_SAMPLE_ASSET_DIR "${ZENO_REPO_ROOT}/samples/sample_game_cpp/assets")
set(ZENO_FEATURE_SAMPLE_ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/assets")

if(NOT TARGET zeno_sdk_cpp)
    add_subdirectory("${ZENO_SDK_DIR}" "${CMAKE_CURRENT_BINARY_DIR}/zeno_sdk_cpp")
endif()

function(zeno_add_sdk_feature_sample target source)
    add_executable(${target} ${source})
    target_compile_features(${target} PRIVATE cxx_std_20)
    target_link_libraries(${target} PRIVATE zeno_sdk_cpp)

    add_custom_command(TARGET ${target} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_FILE:zeno_abi_rust>"
            "$<TARGET_FILE_DIR:${target}>"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ZENO_SHARED_SAMPLE_ASSET_DIR}"
            "$<TARGET_FILE_DIR:${target}>/assets"
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${ZENO_FEATURE_SAMPLE_ASSET_DIR}"
            "$<TARGET_FILE_DIR:${target}>/assets"
    )

    install(TARGETS ${target} RUNTIME DESTINATION bin)
endfunction()

zeno_add_sdk_feature_sample(zeno_sample_2d_input_audio_cpp src/sample_2d_input_audio.cpp)
zeno_add_sdk_feature_sample(zeno_sample_3d_mesh_cpp src/sample_3d_mesh.cpp)
