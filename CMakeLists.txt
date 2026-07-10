# reference: https://github.com/jensroth-git/r3dStarter/blob/main/CMakeLists.txt

cmake_minimum_required(VERSION 3.14...3.30)

project(DrawCAL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

# Build static libs to avoid DLL issues on Windows
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Assimp settings
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(ASSIMP_BUILD_ASSIMP_TOOLS OFF CACHE BOOL "" FORCE)
set(ASSIMP_INSTALL OFF CACHE BOOL "" FORCE)

# r3d settings
set(R3D_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(R3D_RAYLIB_VENDORED OFF CACHE BOOL "" FORCE)
set(R3D_ASSIMP_VENDORED OFF CACHE BOOL "" FORCE)

# 1. Fetch raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0
    GIT_SHALLOW TRUE
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(raylib)

# 2. Fetch raygui
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG 4.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raygui)

# 3. Fetch assimp
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.5
    GIT_SHALLOW TRUE
    OVERRIDE_FIND_PACKAGE
)
FetchContent_MakeAvailable(assimp)

# Make sure assimp::assimp exists for projects that expect it
if(TARGET assimp AND NOT TARGET assimp::assimp)
    add_library(assimp::assimp ALIAS assimp)
endif()

# r3d requires Python for shader/asset embedding
find_package(Python3 COMPONENTS Interpreter REQUIRED)
set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}" CACHE FILEPATH "" FORCE)

# 4. Fetch r3d
FetchContent_Declare(
    r3d
    GIT_REPOSITORY https://github.com/Bigfoot71/r3d.git
    GIT_TAG v0.10.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(r3d)

# Force r3d to see raylib and assimp correctly
if(TARGET r3d)
    target_link_libraries(r3d PUBLIC raylib assimp)

    target_include_directories(r3d PRIVATE
        ${assimp_SOURCE_DIR}/include
        ${assimp_BINARY_DIR}/include
    )
endif()

# MongoDB C++ Driver from vcpkg
find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)

# Project source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp")

add_executable(drawcal_app ${SOURCES})

# Fix raygui MSVC warning/error about C compound literals like (Vector2){...}
if(MSVC)
    target_compile_options(drawcal_app PRIVATE /wd4576)
endif()

target_include_directories(drawcal_app PRIVATE
    src/
    src/data/save_load
    ${r3d_SOURCE_DIR}/include
    ${r3d_SOURCE_DIR}/include/r3d
    ${raylib_SOURCE_DIR}/src
)

target_include_directories(drawcal_app SYSTEM PRIVATE
    ${raygui_SOURCE_DIR}/src
)

target_link_libraries(drawcal_app PRIVATE
    raylib
    r3d
    assimp
    mongo::bsoncxx_shared
    mongo::mongocxx_shared
)

# Output executable as DrawCAL/drawcal/drawcal.exe
set_target_properties(drawcal_app PROPERTIES
    OUTPUT_NAME "drawcal"

    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/drawcal"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/drawcal"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/drawcal"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/drawcal"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/drawcal"
)

# Support for Visual Studio
set_property(TARGET drawcal_app PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:drawcal_app>"
)