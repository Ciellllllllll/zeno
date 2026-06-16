cmake_minimum_required(VERSION 3.24)

project(zeno_external_game LANGUAGES CXX)

find_package(ZENO CONFIG REQUIRED)

add_executable(zeno_external_game
    src/main.cpp
)
target_compile_features(zeno_external_game PRIVATE cxx_std_20)
target_link_libraries(zeno_external_game PRIVATE ZENO::zeno_sdk_cpp)

add_custom_command(TARGET zeno_external_game POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:ZENO::zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_external_game>"
)
