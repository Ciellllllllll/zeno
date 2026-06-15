cmake_minimum_required(VERSION 3.24)

project(zeno_sample_game_cpp LANGUAGES CXX)

add_executable(zeno_sample_game_cpp src/main.cpp)
target_compile_features(zeno_sample_game_cpp PRIVATE cxx_std_20)
