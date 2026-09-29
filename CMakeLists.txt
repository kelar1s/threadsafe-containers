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

add_executable(threadsafe_containers src/main.cpp)

target_include_directories(threadsafe_containers PRIVATE 
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(threadsafe_containers PRIVATE 
    Threads::Threads
)