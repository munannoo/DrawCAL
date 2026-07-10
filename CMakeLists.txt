# reference: https://github.com/jensroth-git/r3dStarter/blob/main/CMakeLists.txt
# the program doesn't build without git_shallow and override_find_package, because r3d has vendored copies of raylib and assimp that conflict with the ones we fetch here.
# git_shallow downloads only the required version and not the history
# override_find_package overrides the find package() calls to make it so that even fetch may be utilised in find

cmake_minimum_required(VERSION 3.14...3.30)
project(DrawCAL)

set(CMAKE_CXX_STANDARD 17)

include(FetchContent)

# Always build static libs to avoid DLL issues on Windows
set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
# assimp knobs (leaner build; keep install/tests off)
set(ASSIMP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
# Configure r3d to use the just-built raylib/assimp instead of its vendored copies
set(R3D_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(R3D_RAYLIB_VENDORED OFF CACHE BOOL "" FORCE)
set(R3D_ASSIMP_VENDORED OFF CACHE BOOL "" FORCE)

# 1. Fetch raylib
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 6.0 
    GIT_SHALLOW TRUE
    OVERRIDE_FIND_PACKAGE ON # Makes it so that find_package(raylib) will find this instead of any system-installed version
)
FetchContent_MakeAvailable(raylib)

# 2. Fetch raygui
FetchContent_Declare(
    raygui
    GIT_REPOSITORY https://github.com/raysan5/raygui.git
    GIT_TAG 4.0
    GIT_SHALLOW TRUE
    OVERRIDE_FIND_PACKAGE ON
)
FetchContent_MakeAvailable(raygui)

# 3. Fetch assimp
FetchContent_Declare(
    assimp
    GIT_REPOSITORY https://github.com/assimp/assimp.git
    GIT_TAG v6.0.5
    GIT_SHALLOW TRUE
    OVERRIDE_FIND_PACKAGE ON # Makes it so that find_package(assimp) will find this instead of any system-installed version
)
FetchContent_MakeAvailable(assimp)

# r3d requires Python for shader/asset embedding
if(NOT DEFINED PYTHON_EXECUTABLE)
    find_package(Python3 COMPONENTS Interpreter REQUIRED)
    set(PYTHON_EXECUTABLE "${Python3_EXECUTABLE}" CACHE FILEPATH "" FORCE)
endif()

# 4. Fetch r3d
FetchContent_Declare(
    r3d
    GIT_REPOSITORY https://github.com/Bigfoot71/r3d.git
    GIT_TAG v0.10.0
    GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(r3d)

# Link r3d to raylib
target_link_libraries(r3d PRIVATE raylib)


# Searches for all .cpp files in the src directory recursively and stores to variable SOURCES
# CONFIGURE_DEPENDS ensures that CMake will re-run if new .cpp files are added to the src directory
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS "src/*.cpp") 

add_executable(drawcal_app ${SOURCES}
    src/features/learning/GuidedMode.hpp)

# Add include directories for the drawcal_app target, PRIVATE means that these include directories are only used for compiling drawcal_app
target_include_directories(drawcal_app PRIVATE 
    src/
    ${r3d_SOURCE_DIR}/include/r3d
    ${raylib_SOURCE_DIR}/src
    ${raygui_SOURCE_DIR}/src
    src/data/save_load
)

target_link_libraries(drawcal_app PRIVATE raylib r3d)

# Output executable as DrawCAL/drawcal/drawcal.exe
set_target_properties(drawcal_app PROPERTIES
    OUTPUT_NAME "drawcal"

    # Set output directory for all build types
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