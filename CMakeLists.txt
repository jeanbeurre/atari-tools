cmake_minimum_required(VERSION 3.10)
project(atari-tools C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(atr atr.c)
add_executable(atr2imd atr2imd.c)
add_executable(imd2atr imd2atr.c)
add_executable(detok detok.c)

