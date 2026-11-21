cmake_minimum_required(VERSION 3.20)
project(threadsafe_containers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
option(BUILD_TESTS "Build functional tests" ON)

if(ENABLE_TSAN)
    message(STATUS "ThreadSanitizer enabled")
    add_compile_options(-fsanitize=thread -g -O1)
    add_link_options(-fsanitize=thread)
endif()

find_package(Threads REQUIRED)

add_library(ts_containers INTERFACE)
target_include_directories(ts_containers INTERFACE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)
target_link_libraries(ts_containers INTERFACE Threads::Threads)

if(BUILD_TESTS)
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG        v1.14.0
    )
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)

    include(GoogleTest)
    enable_testing()

    add_executable(ts_tests
        tests/functional/test_mutex_vector.cpp
        tests/functional/test_lockfree_vector.cpp
        tests/functional/test_mutex_forward_list.cpp
        tests/functional/test_lockfree_forward_list.cpp
        tests/functional/test_mutex_list.cpp
        tests/functional/test_lockfree_list.cpp
    )
    target_link_libraries(ts_tests PRIVATE ts_containers GTest::gtest_main)

    gtest_discover_tests(ts_tests
        PROPERTIES TIMEOUT 300
        DISCOVERY_TIMEOUT 60
    )
endif()

add_executable(performance_benchmarks
    tests/performance/main.cpp
)
target_include_directories(performance_benchmarks PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/performance
)
target_link_libraries(performance_benchmarks PRIVATE ts_containers)

if(NOT ENABLE_TSAN)
    target_compile_options(performance_benchmarks PRIVATE
        $<$<CXX_COMPILER_ID:GNU,Clang,AppleClang>:-O3>
    )
endif()