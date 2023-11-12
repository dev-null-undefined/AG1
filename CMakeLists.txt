cmake_minimum_required(VERSION 3.12)

# Tests
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wno-long-long -D_GLIBCXX_DEBUG")
#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
#set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")

set(CMAKE_CXX_STANDARD 20)

project(ag1_progtest_01_1)
add_executable(ag1_progtest_01_1 pt01/main.cpp)

project(ag1_avl_tree)
add_executable(ag1_avl_tree pt02/avl_tree_tester.cpp)

project(ag1_progtest_02_1)
add_executable(ag1_progtest_02_1 pt02/sample.cpp)

project(ag1_progtest_02_2)
add_executable(ag1_progtest_02_2 pt02-2/main.cpp)

project(ag1_progtest_02_2_mini)
add_executable(ag1_progtest_02_2_mini pt02-2/avl_mini_prog.cpp)

project(ag1_progtest_02_2_testing)
add_executable(ag1_progtest_02_2_testing pt02-2/testing.cpp)

project(ag1_progtest_02_2_giga_chad_tester)
add_executable(ag1_progtest_02_2_giga_chad_tester pt02-2/tester_vec.cpp)

project(AG1)
