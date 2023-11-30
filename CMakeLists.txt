cmake_minimum_required(VERSION 3.12)


set(CMAKE_CXX_STANDARD 20)

project(ag1_testing)
add_executable(ag1_testing testing/smallest-N-numbers.cpp)

# Tests
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wno-long-long -D_GLIBCXX_DEBUG -fno-omit-frame-pointer -rdynamic") #  -O2
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O2") #  -O2
#set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer")

target_link_libraries(ag1_testing benchmark)

execute_process(COMMAND nix build .\#cmake-helper.x86_64-linux.cmake-file --no-link --print-out-paths
        OUTPUT_STRIP_TRAILING_WHITESPACE
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        OUTPUT_VARIABLE cmake-flake-include)

include(${cmake-flake-include})

#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pedantic -Wall -Wextra -Wno-long-long -D_GLIBCXX_DEBUG -fno-omit-frame-pointer -rdynamic") #  -O2
#set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer")

#set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fsanitize=address")
#set(CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fsanitize=address")

project(ag1_progtest_01_1)
add_executable(ag1_progtest_01_1 pt01/main.cpp)

project(ag1_avl_tree)
add_executable(ag1_avl_tree pt02/avl_tree_tester.cpp)

project(ag1_progtest_02_1)
add_executable(ag1_progtest_02_1 pt02/sample.cpp)

project(ag1_progtest_02_2_mini)
add_executable(ag1_progtest_02_2_mini pt02-2/avl_mini_prog.cpp)

project(ag1_progtest_02_2_mini_2)
add_executable(ag1_progtest_02_2_mini_2 pt02-2/avl_pole_mini_prog.cpp)

project(ag1_progtest_02_2_testing)
add_executable(ag1_progtest_02_2_testing pt02-2/testing.cpp)

project(ag1_progtest_02_2_giga_chad_tester)
add_executable(ag1_progtest_02_2_giga_chad_tester pt02-2/tester_vec.cpp)

project(ag1_progtest_topsort)
add_executable(ag1_progtest_topsort pt04/topsort.cpp)

project(ag1_progtest_heap)
add_executable(ag1_progtest_heap pt05/heap.cpp)

project(AG1)
