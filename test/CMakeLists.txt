cmake_minimum_required(VERSION 3.10)
project(JITtest)

set(CMAKE_CXX_STANDARD 17)
SET(GCC_COVERAGE_COMPILE_FLAGS "-g -O0 -ftest-coverage -fprofile-arcs")

set(
  CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -marm ${GCC_COVERAGE_COMPILE_FLAGS}"
)

add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/googletest)

add_executable(JITtest ../parser/parser.cpp ../translator/translator.cpp test.cpp)

target_include_directories(JITtest PUBLIC ${gtest_SOURCE_DIR}/include ${CMAKE_CURRENT_LIST_DIR}/..)

target_link_libraries(JITtest gtest gtest_main)
