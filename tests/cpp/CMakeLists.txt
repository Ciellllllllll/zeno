cmake_minimum_required(VERSION 3.24)

include(FetchContent)

if(MSVC)
    set(gtest_force_shared_crt ON CACHE BOOL "Use the shared MSVC runtime for GoogleTest." FORCE)
endif()
set(INSTALL_GTEST OFF CACHE BOOL "Do not install GoogleTest from the test graph." FORCE)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG f8d7d77c06936315286eb55f8de22cd23c188571 # v1.14.0
)
FetchContent_MakeAvailable(googletest)

include(GoogleTest)

add_executable(zeno_cpp_smoke_tests
    zeno_smoke_tests.cpp
)
target_compile_features(zeno_cpp_smoke_tests PRIVATE cxx_std_20)
target_link_libraries(zeno_cpp_smoke_tests PRIVATE
    zeno_sdk_cpp
    GTest::gtest_main
)

add_custom_command(TARGET zeno_cpp_smoke_tests POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$<TARGET_FILE:zeno_abi_rust>"
        "$<TARGET_FILE_DIR:zeno_cpp_smoke_tests>"
)

gtest_discover_tests(zeno_cpp_smoke_tests
    PROPERTIES LABELS "headless"
)
