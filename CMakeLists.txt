cmake_minimum_required(VERSION 3.10)
set(CPACK_GENERATOR "ZIP" CACHE INTERNAL "package generator")
project(atari-tools C)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_executable(atr atr.c)
add_executable(atr2imd atr2imd.c)
add_executable(imd2atr imd2atr.c)
add_executable(detok detok.c)

set(CPACK_SYSTEM_NAME "${CMAKE_SYSTEM_NAME}-${CMAKE_SYSTEM_PROCESSOR}")
string(TIMESTAMP BUILD_DATE "%Y%m%d")
set(CPACK_ARCHIVE_FILE_NAME "atari-tools-${BUILD_DATE}-${CPACK_SYSTEM_NAME}")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)
install(TARGETS atr atr2imd imd2atr detok RUNTIME DESTINATION .)
include(CPack)