cmake_minimum_required(VERSION 3.10)
project(JIT)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

set(
  CMAKE_CXX_FLAGS
  "${CMAKE_CXX_FLAGS} -marm"
)

set(
  CMAKE_C_FLAGS
  "${CMAKE_CXX_FLAGS} -marm"
)

add_executable(JIT parser/parser.cpp translator/translator.cpp main.c)

target_include_directories(JIT PUBLIC ${CMAKE_CURRENT_LIST_DIR})
