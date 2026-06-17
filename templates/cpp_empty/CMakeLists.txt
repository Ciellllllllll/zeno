cmake_minimum_required(VERSION 3.24)

project(zeno_cpp_empty LANGUAGES CXX)

find_package(ZenoEngine CONFIG REQUIRED)

add_executable(zeno_cpp_empty
    src/main.cpp
)
target_compile_features(zeno_cpp_empty PRIVATE cxx_std_20)
target_link_libraries(zeno_cpp_empty PRIVATE ZenoEngine::zeno_sdk_cpp)

add_custom_command(TARGET zeno_cpp_empty POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${ZenoEngine_BIN_DIR}/$<CONFIG>/zeno_abi.dll"
        "$<TARGET_FILE_DIR:zeno_cpp_empty>"
)
