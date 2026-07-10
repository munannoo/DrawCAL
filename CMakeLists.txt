cmake_minimum_required(VERSION 3.14...3.30)

project(DrawCAL LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

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

# 3. MongoDB C++ Driver
# Install using vcpkg first:
# vcpkg install mongo-cxx-driver:x64-windows
find_package(bsoncxx CONFIG REQUIRED)
find_package(mongocxx CONFIG REQUIRED)

file(GLOB_RECURSE SOURCES "src/*.cpp")

add_executable(drawcal_app ${SOURCES})

target_include_directories(drawcal_app PRIVATE
    src/
    ${raygui_SOURCE_DIR}/src
    src/data/save_load
)

target_link_libraries(drawcal_app PRIVATE
    raylib
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

set_property(TARGET drawcal_app PROPERTY
    VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:drawcal_app>"
)