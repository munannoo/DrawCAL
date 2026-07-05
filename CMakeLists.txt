cmake_minimum_required(VERSION 3.14...3.30)
project(DrawCAL)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# 1. Fetch raylib
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

add_executable(drawcal_app ${SOURCES}
    src/features/learning/GuidedMode.hpp)

target_include_directories(drawcal_app PRIVATE 
    src/
    ${raygui_SOURCE_DIR}/src
    src/data/save_load
)

target_link_libraries(drawcal_app raylib)

# Output executable as DrawCAL/drawcal/drawcal.exe
set_target_properties(drawcal_app PROPERTIES
    OUTPUT_NAME "drawcal"

    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/drawcal"
    RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_SOURCE_DIR}/drawcal"
    RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_SOURCE_DIR}/drawcal"
    RUNTIME_OUTPUT_DIRECTORY_RELWITHDEBINFO "${CMAKE_SOURCE_DIR}/drawcal"
    RUNTIME_OUTPUT_DIRECTORY_MINSIZEREL "${CMAKE_SOURCE_DIR}/drawcal"
)

set_property(TARGET drawcal_app PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:drawcal_app>"
)