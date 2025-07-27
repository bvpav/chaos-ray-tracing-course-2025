cmake_minimum_required(VERSION 3.10)

project(crt_renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

file(GLOB_RECURSE CRT_CORE_SOURCES
    "src/core/*.cpp"
    "src/core/*.h"
)
add_library(crt_core STATIC ${CRT_CORE_SOURCES})

target_include_directories(crt_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/rapidjson/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb)

file(GLOB_RECURSE CRT_STANDALONE_SOURCES
    "src/standalone/*.cpp"
    "src/standalone/*.h"
)
add_executable(${PROJECT_NAME} ${CRT_STANDALONE_SOURCES})

target_link_libraries(${PROJECT_NAME} PRIVATE crt_core)