# Reference:
# https://github.com/jensroth-git/r3dStarter/blob/main/CMakeLists.txt

cmake_minimum_required(VERSION 3.14...3.30)

project(DrawCAL LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

# Build static libraries to avoid DLL issues on Windows
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)

# Fetch raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG master
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(raylib)

# Fetch raygui
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG 4.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(raygui)

# Find all source files
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
)

# Create executable
add_executable(drawcal_app ${SOURCES})

# Include directories
target_include_directories(drawcal_app PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
    "${CMAKE_SOURCE_DIR}/src/data/save_load"
    "${raylib_SOURCE_DIR}/src"
)

target_include_directories(drawcal_app SYSTEM PRIVATE
    "${raygui_SOURCE_DIR}/src"
)

# Link libraries
target_link_libraries(drawcal_app PRIVATE
    raylib
)

# Output executable as DrawCAL/drawcal/drawcal.exe
set_target_properties(drawcal_app PROPERTIES
    OUTPUT_NAME "drawcal"

    RUNTIME_OUTPUT_DIRECTORY
        "${CMAKE_SOURCE_DIR}/drawcal"

    RUNTIME_OUTPUT_DIRECTORY_DEBUG
        "${CMAKE_SOURCE_DIR}/drawcal"

    RUNTIME_OUTPUT_DIRECTORY_RELEASE
        "${CMAKE_SOURCE_DIR}/drawcal"

    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO
        "${CMAKE_SOURCE_DIR}/drawcal"

    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL
        "${CMAKE_SOURCE_DIR}/drawcal"
)

# Visual Studio debugger working directory
set_property(
    TARGET drawcal_app
    PROPERTY VS_DEBUGGER_WORKING_DIRECTORY
    "$<TARGET_FILE_DIR:drawcal_app>"
)