cmake_minimum_required(VERSION 3.20)
project(threadsafe_containers LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

option(ENABLE_TSAN "Enable ThreadSanitizer" OFF)
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

add_executable(threadsafe_containers src/main.cpp)
target_link_libraries(threadsafe_containers PRIVATE ts_containers)

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
    tests/test_mutex_vector.cpp
    tests/test_lockfree_vector.cpp
    tests/test_lockfree_forward_list.cpp
)
target_link_libraries(ts_tests PRIVATE ts_containers GTest::gtest_main)

gtest_discover_tests(ts_tests
    PROPERTIES TIMEOUT 300
    DISCOVERY_TIMEOUT 60
)
