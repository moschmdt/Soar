# SpawnDebugger Test - Quick Build Guide

## Building

Since Soar should be in the CMake path, you need to tell CMake where to find it:

### Option 1: Using CMAKE_PREFIX_PATH

```bash
cd Tests/Debugger
mkdir -p build && cd build

# Point to Soar build directory
cmake -DCMAKE_PREFIX_PATH=../../build/Debug ..
cmake --build .
```

### Option 2: Using build directory directly

```bash
cd Tests/Debugger
mkdir -p build && cd build

# Let CMake find Soar in the build tree
cmake -Dsoar_DIR=../../build/Debug ..
cmake --build .
```

### Option 3: Using installed Soar

```bash
# If Soar is installed
cd Tests/Debugger
mkdir -p build && cd build

cmake -DCMAKE_PREFIX_PATH=../../install ..
cmake --build .
```

## Running

```bash
# Use system-installed debugger
./spawn_debugger_test

# Use explicit path
./spawn_debugger_test ../../build/Debug/QtDebugger/SoarDebugger
```

## What Changed

The CMakeLists.txt is now much simpler - it just uses `find_package(soar REQUIRED)` and links to `soar::soar_lib`. All dependencies (SQLite3, Boost, etc.) are handled automatically by Soar's CMake configuration.
