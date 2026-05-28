cmake_minimum_required(VERSION 3.14...3.30)
project(DrawCAL)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# 1. Fetch raylib (Switching to master bypasses the version error entirely)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG master 
)
FetchContent_MakeAvailable(raylib)

# 2. Fetch raygui
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG master
)
FetchContent_MakeAvailable(raygui)

file(GLOB_RECURSE SOURCES "src/*.cpp")
add_executable(drawcal_app ${SOURCES})

target_include_directories(drawcal_app PRIVATE 
    src/
    ${raygui_SOURCE_DIR}/src
)

target_link_libraries(drawcal_app raylib)