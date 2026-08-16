# CMake is the "build system" for C++ projects. It's the rough equivalent of
# Maven or Gradle in Java: it decides which files get compiled, in what order,
# and which libraries get linked in.
#
# You do not run the compiler yourself. You run CMake, and CMake runs the
# compiler for you.

cmake_minimum_required(VERSION 3.16)
project(greyproject1 CXX)

# Use the C++17 standard. (C++ has dated "editions" - 11, 14, 17, 20, 23.
# 17 is a comfortable, widely-supported baseline.)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
# FetchContent downloads a library straight from GitHub at build time and
# compiles it alongside our code. This means raylib is NOT checked into this
# repo and nobody has to install it by hand - cloning + building is enough.
include(FetchContent)

# Don't build raylib's own examples/games; we only want the library itself.
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.0          # pinned so the build stays reproducible
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib)

# ---------------------------------------------------------------------------
# Our game
# ---------------------------------------------------------------------------
# Add new .cpp files to this list as the project grows.
add_executable(game
    src/main.cpp
)

target_link_libraries(game PRIVATE raylib)

# Quality-of-life for Visual Studio: make "game" the project that runs when you
# hit F5, and run it from the repo root so file paths like "assets/player.png"
# resolve the way you'd expect.
if(MSVC)
    set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT game)
    set_property(TARGET game PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
endif()
