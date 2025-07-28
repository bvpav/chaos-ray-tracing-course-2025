cmake_minimum_required(VERSION 3.10)

project(crt_renderer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

option(BUILD_STANDALONE        "Build the standalone executable (no Python required)"                             ON)
option(BUILD_PYTHON            "Build the Python extension module"                                                OFF)
option(BUILD_BLENDER_EXTENSION "Build the Blender extension package (requires Python 3.11 development libraries)" OFF)
    
if (BUILD_BLENDER_EXTENSION AND NOT BUILD_PYTHON)
    set(BUILD_PYTHON ON
        CACHE BOOL "Build the Python extension module" FORCE)
    message(STATUS
            "Enabling BUILD_PYTHON because BUILD_BLENDER_EXTENSION is ON")
endif()

file(GLOB_RECURSE CRT_CORE_SOURCES
    "src/core/*.cpp"
    "src/core/*.h"
)
add_library(crt_core STATIC ${CRT_CORE_SOURCES})

if (BUILD_PYTHON)
    # Python requires PIC
    set_property(TARGET crt_core PROPERTY POSITION_INDEPENDENT_CODE ON)
endif()

target_include_directories(crt_core
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/rapidjson/include
        ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb)

if (BUILD_STANDALONE)
    file(GLOB_RECURSE CRT_STANDALONE_SOURCES
        "src/standalone/*.cpp"
        "src/standalone/*.h"
    )
    add_executable(${PROJECT_NAME} ${CRT_STANDALONE_SOURCES})

    target_link_libraries(${PROJECT_NAME} PRIVATE crt_core)
endif()

if (BUILD_PYTHON)
    find_package(Python3 3.11 EXACT REQUIRED
        COMPONENTS Interpreter Development.Module)

    file(GLOB_RECURSE CRT_PYTHON_SOURCES
        "src/python/*.cpp"
        "src/python/*.h"
    )

    Python3_add_library(_crt MODULE WITH_SOABI ${CRT_PYTHON_SOURCES})

    target_link_libraries(_crt PRIVATE crt_core)

    set(CRT_PYTHON_MODULE_FILE "$<TARGET_FILE:_crt>")
endif()

if (BUILD_BLENDER_EXTENSION)
    add_custom_target(crt_blender_extension ALL
        DEPENDS _crt
        COMMENT "Creating Blender extension package"
    )

    set(CRT_BLENDER_EXTENSION_STAGING_DIR "${CMAKE_CURRENT_BINARY_DIR}/blender_extension")
    set(CRT_BLENDER_EXTENSION_ZIP "${CMAKE_CURRENT_BINARY_DIR}/crt_blender_extension.zip")

    add_custom_command(
        TARGET crt_blender_extension
        COMMAND ${CMAKE_COMMAND} -E make_directory ${CRT_BLENDER_EXTENSION_STAGING_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_CURRENT_SOURCE_DIR}/src/blender ${CRT_BLENDER_EXTENSION_STAGING_DIR}
        COMMAND ${CMAKE_COMMAND} -E copy ${CRT_PYTHON_MODULE_FILE} ${CRT_BLENDER_EXTENSION_STAGING_DIR}
        
        # Create zip using Python (which is already a requirement)
        COMMAND ${Python3_EXECUTABLE} -c "import zipfile, os; zip_file = zipfile.ZipFile('${CRT_BLENDER_EXTENSION_ZIP}', 'w'); [zip_file.write(os.path.join(root, file), os.path.relpath(os.path.join(root, file), '${CRT_BLENDER_EXTENSION_STAGING_DIR}')) for root, _, files in os.walk('${CRT_BLENDER_EXTENSION_STAGING_DIR}') for file in files]; zip_file.close()"
        
        VERBATIM
    )
endif()
