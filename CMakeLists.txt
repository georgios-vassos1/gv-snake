cmake_minimum_required(VERSION 3.14)
project(snake VERSION 0.3 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

# ── Game library (shared between the executable and the tests) ────────────────
add_library(snake_core STATIC
    src/Point.cpp
    src/List.cpp
    src/ShitList.cpp
    src/Game.cpp
    src/HighScore.cpp
    src/TerminalRenderer.cpp
    src/mygetch.cpp
    src/QAgent.cpp
    src/AgentRenderer.cpp
    src/HamiltonianAgent.cpp
)
target_include_directories(snake_core PUBLIC include)
target_compile_options(snake_core PRIVATE -Wall -Wextra -O2)
target_link_libraries(snake_core PUBLIC Threads::Threads)

# ── Optional SDL2 graphics backend ────────────────────────────────────────────
find_package(SDL2 QUIET)
if(SDL2_FOUND)
    message(STATUS "SDL2 found — graphics mode enabled")
    target_sources(snake_core PRIVATE src/GraphicsRenderer.cpp)
    target_compile_definitions(snake_core PUBLIC GRAPHICS_AVAILABLE)
    target_include_directories(snake_core PUBLIC ${SDL2_INCLUDE_DIRS})
    target_link_libraries(snake_core PUBLIC SDL2::SDL2)
else()
    message(STATUS "SDL2 not found — terminal-only build")
endif()

# ── Optional libtorch (DQN agent) ─────────────────────────────────────────────
find_package(Torch QUIET)
if(Torch_FOUND)
    message(STATUS "libtorch found — DQN agent enabled")
    add_library(snake_dqn STATIC
        src/DQNAgent.cpp
        src/DQNAgentRenderer.cpp
    )
    target_include_directories(snake_dqn PUBLIC include)
    target_compile_options(snake_dqn PRIVATE -Wall -Wextra -O2)
    set_target_properties(snake_dqn PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
    target_compile_definitions(snake_dqn PUBLIC DQN_AVAILABLE)
    target_link_libraries(snake_dqn PUBLIC snake_core ${TORCH_LIBRARIES})
    target_include_directories(snake_dqn PUBLIC ${TORCH_INCLUDE_DIRS})
else()
    message(STATUS "libtorch not found — DQN agent disabled")
endif()

# ── Main executable ───────────────────────────────────────────────────────────
add_executable(snake src/main.cpp)
target_compile_options(snake PRIVATE -Wall -Wextra -O2)
target_link_libraries(snake PRIVATE snake_core)

if(Torch_FOUND)
    target_link_libraries(snake PRIVATE snake_dqn)
    set_target_properties(snake PROPERTIES
        CXX_STANDARD 17
        CXX_STANDARD_REQUIRED ON
    )
endif()

# ── Tests ─────────────────────────────────────────────────────────────────────
enable_testing()

add_executable(point_test tests/Point_test.cpp)
target_link_libraries(point_test PRIVATE snake_core)
add_test(NAME point_test COMMAND point_test)

add_executable(list_test tests/List_test.cpp)
target_link_libraries(list_test PRIVATE snake_core)
add_test(NAME list_test COMMAND list_test)

add_executable(score_test tests/score_test.cpp)
target_link_libraries(score_test PRIVATE snake_core)
add_test(NAME score_test COMMAND score_test)

add_executable(qagent_test tests/qagent_test.cpp)
target_link_libraries(qagent_test PRIVATE snake_core)
add_test(NAME qagent_test COMMAND qagent_test)

add_executable(hamiltonian_test tests/hamiltonian_test.cpp)
target_link_libraries(hamiltonian_test PRIVATE snake_core)
add_test(NAME hamiltonian_test COMMAND hamiltonian_test)

if(Torch_FOUND)
    add_executable(dqn_test tests/dqn_test.cpp)
    target_link_libraries(dqn_test PRIVATE snake_dqn)
    set_target_properties(dqn_test PROPERTIES CXX_STANDARD 17 CXX_STANDARD_REQUIRED ON)
    add_test(NAME dqn_test COMMAND dqn_test)
endif()
