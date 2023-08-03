cmake_minimum_required(VERSION 3.17)
project(breadscript)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED True)

file(GLOB CITH_SOURCES "cith/*.c")
set(CITH_INCLUDES ${CMAKE_CURRENT_SOURCE_DIR}/cith)

file(GLOB SOURCE "*.c")

add_executable(breadscript ${SOURCE} ${CITH_SOURCES})

target_include_directories(breadscript PUBLIC ${CMAKE_CURRENT_SOURCE_DIR} ${CITH_INCLUDES})
