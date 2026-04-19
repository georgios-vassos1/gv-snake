# Snake

A terminal-based Snake game written in C++11 with an optional SDL2 graphics mode, a tabular Q-learning agent that can be trained to play autonomously, and a Hamiltonian-cycle agent that achieves perfect scores.

## Features

- Smooth in-place terminal rendering via ANSI escape codes
- Optional SDL2 graphics mode (`--graphics`)
- Wrapping borders — the snake passes through walls and reappears on the other side
- Game over only on self-collision
- Pause/resume with `Space`
- Persistent high score
- **Tabular Q-learning agent** — train in minutes, watch it play in terminal or SDL2
- **Hamiltonian-cycle agent** — algorithmic solver that fills the entire board (perfect 314/314 scores)

## Requirements

| Dependency | Version | Notes |
|---|---|---|
| C++ compiler | C++11 | GCC or Clang |
| CMake | 3.14+ | |
| POSIX threads | — | standard on macOS and Linux |
| SDL2 | any | optional — enables `--graphics` |

Install SDL2 on macOS: `brew install sdl2`  
Install SDL2 on Ubuntu/Debian: `sudo apt install libsdl2-dev`

## Quick start

```bash
make          # configure + build
make run      # play yourself (terminal)
make test     # run unit tests
```

## Build

The project uses CMake under the hood. The Makefile is a convenience wrapper — use whichever you prefer.

**Via Makefile (recommended):**
```bash
make build
```

**Via CMake directly:**
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
```

## Human play

```bash
make run              # terminal mode
make run-graphics     # SDL2 window (requires SDL2)
```

### Controls

| Key | Action |
|---|---|
| `w` / `↑` | Move up |
| `s` / `↓` | Move down |
| `a` / `←` | Move left |
| `d` / `→` | Move right |
| `Space` | Pause / resume |
| `Escape` | Quit (graphics mode) |

## RL agent

The agent uses **tabular Q-learning** with a hand-crafted 32 768-state feature vector:

| Feature | Bits | Values |
|---|---|---|
| Current direction | 2 | 4 (N/S/E/W) |
| Danger straight 1/2/3 steps | 3 | 8 |
| Danger left 1/2/3 steps | 3 | 8 |
| Danger right 1/2/3 steps | 3 | 8 |
| Food X distance bin | 2 | 4 (far-above / near-above / near-below / far-below) |
| Food Y distance bin | 2 | 4 (far-left / near-left / near-right / far-right) |

The state space fits in a 512 KiB lookup table, so no neural network is needed.

At play time a **flood-fill safety filter** overrides the greedy action when it
would leave the snake's reachable area smaller than its body length, preventing
self-enclosure.

### Train

```bash
make train                        # 10 000 episodes → build/qtable.bin
make train EPISODES=50000         # longer run
make train QTABLE=/tmp/my.bin     # custom output path
```

Training progress is printed every 5% of episodes:

```
Training: 10000 episodes on 20x20 grid
  Episode  500/10000 | avg:   1.23 | best:   8 | ε: 0.3162
  Episode 1000/10000 | avg:   4.87 | best:  22 | ε: 0.1000
  ...
  Episode 10000/10000 | avg:  38.14 | best:  71 | ε: 0.0100
Q-table saved to build/qtable.bin
```

### Watch the rollout

```bash
make play              # terminal renderer
make play-graphics     # SDL2 window (requires SDL2)
```

Custom Q-table path:
```bash
make play QTABLE=/tmp/my.bin
```

The SDL2 window title shows live stats: `Snake [AI]  Score: 42  Best: 71  Ep: 7`.  
Press **Escape** or close the window to quit.

## Hamiltonian cycle agent

The agent precomputes a **Hamiltonian cycle** over the 18×18 interior grid — a path that visits every cell exactly once before returning to the start. Following the cycle guarantees the snake never traps itself.

To reach food faster, the agent takes **greedy shortcuts**: at each step it looks for a forward jump in cycle order that reduces the remaining distance to food. A shortcut is only taken when it passes three safety checks:

1. The destination is not occupied by the snake body.
2. The jump stays within the safe forward window (no body segment between the head and the shortcut destination).
3. A flood-fill from the destination confirms at least 2× the body length of reachable space.

This combination achieves **perfect scores** (314/314 fruits on the 20×20 grid) without any training.

```bash
make hamiltonian              # terminal renderer
make hamiltonian-graphics     # SDL2 window (requires SDL2)
```

## Tests

```bash
make test
```

Four test suites are run via CTest:

| Suite | What it checks |
|---|---|
| `point_test` | Point arithmetic and movement |
| `list_test` | Doubly-linked list operations |
| `score_test` | Fruit collection and high-score persistence |
| `qagent_test` | State encoding, Q-update math, epsilon decay, save/load, training convergence |

## Makefile targets

| Target | Description |
|---|---|
| `make` / `make build` | Configure (if needed) and compile |
| `make test` | Run all unit tests |
| `make train` | Train the RL agent |
| `make play` | Watch agent in terminal |
| `make play-graphics` | Watch agent in SDL2 window |
| `make hamiltonian` | Watch Hamiltonian agent (terminal) |
| `make hamiltonian-graphics` | Watch Hamiltonian agent (SDL2) |
| `make run` | Human play (terminal) |
| `make run-graphics` | Human play (SDL2) |
| `make clean` | Remove compiled objects |
| `make distclean` | Remove entire `build/` directory |

## Project structure

```
.
├── Makefile
├── CMakeLists.txt
├── include/
│   ├── Game.hpp            game state and tick interface
│   ├── IRenderer.hpp       renderer base class
│   ├── TerminalRenderer.hpp
│   ├── GraphicsRenderer.hpp  (SDL2, compiled only when available)
│   ├── AgentRenderer.hpp     terminal renderer driven by Q-agent
│   ├── QAgent.hpp            tabular Q-learning agent
│   ├── HamiltonianAgent.hpp  Hamiltonian-cycle agent with greedy shortcuts
│   ├── ShitList.hpp        snake body (doubly-linked list wrapper)
│   ├── List.hpp            generic doubly-linked list
│   ├── Point.hpp           2-D grid coordinate
│   ├── HighScore.hpp       score persistence
│   └── mygetch.hpp         raw terminal input
├── src/
│   ├── main.cpp              entry point and CLI flag dispatch
│   ├── Game.cpp
│   ├── QAgent.cpp
│   ├── HamiltonianAgent.cpp
│   ├── AgentRenderer.cpp
│   ├── TerminalRenderer.cpp
│   ├── GraphicsRenderer.cpp
│   └── ...
└── tests/
    ├── qagent_test.cpp
    ├── score_test.cpp
    ├── Point_test.cpp
    └── List_test.cpp
```
