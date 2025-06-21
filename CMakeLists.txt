cmake_minimum_required(VERSION 3.10)

project(crt_renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE SOURCES
    "src/*.cpp"
    "src/*.h"
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE "src")
