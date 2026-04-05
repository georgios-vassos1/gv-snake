# Snake

A terminal-based Snake game written in C++11 using POSIX threads.

## Features

- Smooth in-place terminal rendering via ANSI escape codes
- Wrapping borders — the snake passes through walls and reappears on the opposite side
- Game over only on self-collision
- Pause/resume with `Space`
- Fruit collection grows the snake

## Controls

| Key | Action |
|-----|--------|
| `w` | Move up |
| `s` | Move down |
| `a` | Move left |
| `d` | Move right |
| `Space` | Pause / resume |

## Requirements

- C++11 compiler (GCC or Clang)
- CMake 3.14 or later
- POSIX threads (standard on macOS and Linux)

## Build

```bash
cmake -S . -B build
cmake --build build
```

## Run

```bash
./build/snake
```

## Test

```bash
ctest --test-dir build
```

## Project structure

```
.
├── CMakeLists.txt
├── main.cpp
├── include/        # Public headers
├── src/            # Implementation
└── tests/          # Unit tests
```
