cmake_minimum_required(VERSION 3.31)
project(TR_Programming_Language C)

set(CMAKE_C_STANDARD 11)

# Moves the input file in root directory to the binary (working) directory so CMake can find it
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/front1.in DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/front2.in DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/front3.in DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/front4.in DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/front1.in ${CMAKE_CURRENT_BINARY_DIR}/front.in COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/front2.in ${CMAKE_CURRENT_BINARY_DIR}/front.in COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/front3.in ${CMAKE_CURRENT_BINARY_DIR}/front.in COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/front4.in ${CMAKE_CURRENT_BINARY_DIR}/front.in COPYONLY)

add_executable(TR_Programming_Language front.c)
