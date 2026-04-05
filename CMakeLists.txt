cmake_minimum_required(VERSION 3.14)
project(snake VERSION 0.2 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

# ── Game library (shared between the executable and the tests) ────────────────
add_library(snake_core STATIC
    src/Point.cpp
    src/List.cpp
    src/ShitList.cpp
    src/Terrain.cpp
    src/mygetch.cpp
)
target_include_directories(snake_core PUBLIC include)
target_compile_options(snake_core PRIVATE -Wall -Wextra -O2)
target_link_libraries(snake_core PUBLIC Threads::Threads)

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(snake src/main.cpp)
target_compile_options(snake PRIVATE -Wall -Wextra -O2)
target_link_libraries(snake PRIVATE snake_core)

# ── Tests ─────────────────────────────────────────────────────────────────────
enable_testing()

add_executable(point_test tests/Point_test.cpp)
target_link_libraries(point_test PRIVATE snake_core)
add_test(NAME point_test COMMAND point_test)

add_executable(list_test tests/List_test.cpp)
target_link_libraries(list_test PRIVATE snake_core)
add_test(NAME list_test COMMAND list_test)
