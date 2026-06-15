cmake_minimum_required(VERSION 3.24)

project(zeno_native LANGUAGES CXX)

add_library(zeno_native INTERFACE)
target_compile_features(zeno_native INTERFACE cxx_std_20)
target_include_directories(zeno_native INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
