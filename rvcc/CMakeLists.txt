cmake_minimum_required(VERSION 3.13.5)

project(rvcc)

add_executable(rvcc
  main.c
  tokenize.c
  parse.c
  type.c
  codegen.c
)

SET(CMAKE_C_FLAGS "-std=c11 -g -fno-common")
