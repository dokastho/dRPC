cmake_minimum_required(VERSION 3.22)

project(drpc_cmake)

set(CMAKE_CXX_COMPILER "clang++")
set(CMAKE_C_COMPILER "clang")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wconversion -Wall -Werror -Wextra -pedantic")
set(THREADS_PREFER_PTHREAD_FLAG ON)

if(CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3 -DDEBUG")
endif()

if(CMAKE_BUILD_TYPE STREQUAL "Fast")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -ofast")
endif()

file(GLOB SOURCES "src/*.cpp")
list(FILTER SOURCES EXCLUDE REGEX "src/test_.*\\.cpp")

add_library(drpc SHARED ${SOURCES})

target_link_libraries(drpc ${LLVM_LIBRARIES})
find_package(Threads REQUIRED)
target_link_libraries(drpc PRIVATE Threads::Threads)

file(GLOB TEST_SOURCES "src/test_*.cpp")

if(CMAKE_BUILD_TYPE STREQUAL "Test")
    add_executable(drpc_tests ${TEST_SOURCES})
    target_link_libraries(drpc_tests ${LLVM_LIBRARIES})
endif()
