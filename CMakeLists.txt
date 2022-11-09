cmake_minimum_required(VERSION 3.12)

# Tests
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wno-long-long -D_GLIBCXX_DEBUG")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

set(CMAKE_CXX_STANDARD 17)

project(ag1_progtest_01_1)
add_executable(ag1_progtest_01_1 pt01/main.cpp)
