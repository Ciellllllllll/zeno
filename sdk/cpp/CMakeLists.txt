cmake_minimum_required(VERSION 3.24)

project(zeno_sdk_cpp LANGUAGES CXX)

add_library(zeno_sdk_cpp INTERFACE)
target_compile_features(zeno_sdk_cpp INTERFACE cxx_std_20)
target_include_directories(zeno_sdk_cpp INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
