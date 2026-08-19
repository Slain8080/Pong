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

# ---------------------------------------------------------------------------
# Web build (Emscripten)
# ---------------------------------------------------------------------------
# Emscripten is a compiler that targets the browser instead of Windows/macOS.
# It turns C++ into WebAssembly (.wasm) plus a bit of JavaScript glue (.js).
#
# You never set this block off by hand. It activates only when you configure
# the build through `emcmake`, which quietly points CMake at Emscripten's
# compiler and defines EMSCRIPTEN for us. Desktop builds skip it entirely, so
# adding this changes nothing about how you build today.
#
# raylib handles its own half: it sees Emscripten, switches PLATFORM to Web on
# its own, and passes us the browser-specific flags it needs (-sUSE_GLFW=3)
# through target_link_libraries. So all that's left is our half.
if(EMSCRIPTEN)
    # Normally the linker emits an executable. Here we ask for ".html", which
    # makes Emscripten ALSO write a small page that loads the .js and .wasm and
    # gives them a canvas to draw on. Without this you'd get the pieces of a
    # web game but no page to run them in.
    set(CMAKE_EXECUTABLE_SUFFIX ".html")

    target_link_options(game PRIVATE
        # A web build asks the browser for its memory up front. Rather than
        # guessing a fixed amount, let it grow on demand.
        -sALLOW_MEMORY_GROWTH=1

        # Emscripten's default stack is small (64 KB). Games routinely want
        # more, and overflowing it fails in confusing ways. Cheap insurance.
        -sSTACK_SIZE=1mb
    )

    # When the game gains assets (sprites, sounds), they must be bundled INTO
    # the build - a browser can't read your hard drive. That's one more line:
    #     --preload-file "${CMAKE_CURRENT_SOURCE_DIR}/assets@assets"
    # Not needed yet, since Pong draws everything with shapes.
endif()
