cmake_minimum_required(VERSION 3.10)
project(ChessEngine CXX)

set(CMAKE_CXX_STANDARD 17)

add_library(chess_engine
    src/board.cpp
    src/moveGenerator.cpp
    src/utils.cpp
    src/eval.cpp
    src/search.cpp
)

# Include headers for the library
 target_include_directories(chess_engine PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src)

add_executable(chess_main src/main.cpp)
target_link_libraries(chess_main chess_engine)

# Tests
enable_testing()
add_executable(test_movegenerator tests/test_movegenerator.cpp)
target_link_libraries(test_movegenerator chess_engine)
add_test(NAME test_movegenerator COMMAND test_movegenerator)
set_tests_properties(test_movegenerator PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_executable(test_all_perfts tests/test_all_perfts.cpp)
target_link_libraries(test_all_perfts chess_engine)
add_test(NAME test_all_perfts COMMAND test_all_perfts)
set_tests_properties(test_all_perfts PROPERTIES WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

# Optional benchmark based on gperftools. Disabled by default because it can
# take a long time to run. Uncomment the following lines to build the
# benchmark when gperftools is available.
# find_package(Threads REQUIRED)
# find_library(GPERFTOOLS_LIB profiler)
# if(GPERFTOOLS_LIB)
#   message(STATUS "Found gperftools: ${GPERFTOOLS_LIB}")
#   add_executable(test_perft tests/test_perft.cpp)
#   target_link_libraries(test_perft chess_engine ${GPERFTOOLS_LIB} Threads::Threads)
#   add_test(NAME test_perft COMMAND test_perft)
# endif()
