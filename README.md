# greyproject1

A game written in C++ with [raylib](https://www.raylib.com/).

## What you need installed

| Tool | Why | Windows |
| --- | --- | --- |
| A C++ compiler | Turns the code into a program | [Visual Studio Community](https://visualstudio.microsoft.com/downloads/) with the **Desktop development with C++** workload |
| [CMake](https://cmake.org/download/) 3.16+ | Drives the build | Installer, or bundled with Visual Studio |
| [Git](https://git-scm.com/) | Version control | Installer, or [GitHub Desktop](https://desktop.github.com/) |

You do **not** need to download raylib. CMake fetches and builds it automatically
the first time you configure the project.

## Building

From the repo root:

```bash
cmake -S . -B build          # configure (slow the first time - it downloads and builds raylib)
cmake --build build          # compile our code (fast after that)
```

Then run it:

```bash
./build/Debug/game.exe       # Windows / Visual Studio
./build/game                 # Linux / macOS
```

The first configure takes a few minutes. Every build after that takes seconds,
because raylib is already compiled and only changed files get recompiled.

### If you'd rather use the Visual Studio GUI

`cmake -S . -B build` generates `build/greyproject1.sln`. Open it, and press
F5 to build and run with the debugger attached.

## Layout

```
CMakeLists.txt    build configuration - add new .cpp files here
src/main.cpp      the game
build/            generated output; not committed (see .gitignore)
```

## Useful links

- [raylib cheatsheet](https://www.raylib.com/cheatsheet/cheatsheet.html) — every function on one page. Keep this open.
- [raylib examples](https://www.raylib.com/examples.html) — runnable in the browser, with source.
- [learncpp.com](https://www.learncpp.com/) — the best free C++ course.
