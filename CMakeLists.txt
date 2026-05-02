cmake_minimum_required(VERSION 3.14...3.30)
project(DrawCAL)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
)

FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(drawcal_app ${SOURCES})

target_include_directories(drawcal_app PRIVATE src/)

target_link_libraries(drawcal_app raylib)