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

# Required by Python
set_property(TARGET crt_core PROPERTY POSITION_INDEPENDENT_CODE ON)

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

find_package(Python3 3.11 EXACT REQUIRED
    COMPONENTS Interpreter Development.Module)

file(GLOB_RECURSE CRT_PYTHON_SOURCES
    "src/python/*.cpp"
    "src/python/*.h"
)

Python3_add_library(_crt MODULE WITH_SOABI ${CRT_PYTHON_SOURCES})

target_link_libraries(_crt PRIVATE crt_core)

add_custom_target(crt_blender_extension ALL
    DEPENDS _crt
    COMMENT "Creating Blender extension package"
)

set(CRT_MODULE_FILE "$<TARGET_FILE:_crt>")

set(CRT_BLENDER_EXTENSION_STAGING_DIR "${CMAKE_CURRENT_BINARY_DIR}/blender_extension")
set(CRT_BLENDER_EXTENSION_ZIP "${CMAKE_CURRENT_BINARY_DIR}/crt_blender_extension.zip")

add_custom_command(
    TARGET crt_blender_extension
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CRT_BLENDER_EXTENSION_STAGING_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src/blender ${CRT_BLENDER_EXTENSION_STAGING_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CRT_MODULE_FILE} ${CRT_BLENDER_EXTENSION_STAGING_DIR}
    
    # Create zip using Python (which is already a requirement)
    COMMAND ${Python3_EXECUTABLE} -c "import zipfile, os; zip_file = zipfile.ZipFile('${CRT_BLENDER_EXTENSION_ZIP}', 'w'); [zip_file.write(os.path.join(root, file), os.path.relpath(os.path.join(root, file), '${CRT_BLENDER_EXTENSION_STAGING_DIR}')) for root, _, files in os.walk('${CRT_BLENDER_EXTENSION_STAGING_DIR}') for file in files]; zip_file.close()"
    
    COMMENT "Creating Blender extension package"
    VERBATIM
)