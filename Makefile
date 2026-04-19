BUILD_DIR  := build
CMAKE      := cmake
NPROC      := $(shell nproc 2>/dev/null || sysctl -n hw.logicalcpu 2>/dev/null || echo 4)
SNAKE      := $(BUILD_DIR)/snake
QTABLE     := $(BUILD_DIR)/qtable.bin
DQN_MODEL  := $(BUILD_DIR)/dqn_model.pt
EPISODES   ?= 10000
DQN_EPISODES ?= 5000

.PHONY: all configure build test train play play-graphics hamiltonian hamiltonian-graphics dqn-train dqn-play dqn-play-graphics run run-graphics clean distclean help

# ── Default target ────────────────────────────────────────────────────────────

all: build

# ── CMake configure ───────────────────────────────────────────────────────────

configure: $(BUILD_DIR)/CMakeCache.txt

$(BUILD_DIR)/CMakeCache.txt:
	$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=Release

# ── Build ─────────────────────────────────────────────────────────────────────

build: configure
	$(CMAKE) --build $(BUILD_DIR) --parallel $(NPROC)

# ── Tests ─────────────────────────────────────────────────────────────────────

test: build
	ctest --test-dir $(BUILD_DIR) --output-on-failure

# ── RL agent: train ───────────────────────────────────────────────────────────
# Override episode count:  make train EPISODES=50000
# Override Q-table path:   make train QTABLE=/tmp/my.bin

train: build
	$(SNAKE) --train $(EPISODES) --qtable $(QTABLE)

# ── RL agent: watch rollout (terminal) ───────────────────────────────────────

play: build
	$(SNAKE) --play --qtable $(QTABLE)

# ── RL agent: watch rollout (SDL2 graphics) ───────────────────────────────────

play-graphics: build
	$(SNAKE) --play --graphics --qtable $(QTABLE)

# ── Hamiltonian cycle agent ───────────────────────────────────────────────────

hamiltonian: build
	$(SNAKE) --hamiltonian

hamiltonian-graphics: build
	$(SNAKE) --hamiltonian --graphics

# ── DQN agent ─────────────────────────────────────────────────────────────────

dqn-train: build
	$(SNAKE) --dqn-train $(DQN_EPISODES) --dqn-model $(DQN_MODEL)

dqn-play: build
	$(SNAKE) --dqn-play --dqn-model $(DQN_MODEL)

dqn-play-graphics: build
	$(SNAKE) --dqn-play --graphics --dqn-model $(DQN_MODEL)

# ── Human play ────────────────────────────────────────────────────────────────

run: build
	$(SNAKE)

run-graphics: build
	$(SNAKE) --graphics

# ── Housekeeping ──────────────────────────────────────────────────────────────

clean:
	$(CMAKE) --build $(BUILD_DIR) --target clean 2>/dev/null || true

distclean:
	rm -rf $(BUILD_DIR)

# ── Help ──────────────────────────────────────────────────────────────────────

help:
	@echo "Targets:"
	@echo "  make                   build the project (default)"
	@echo "  make build             configure + compile"
	@echo "  make test              run all unit tests"
	@echo "  make train             train RL agent (EPISODES=10000 by default)"
	@echo "  make play              watch trained agent in terminal"
	@echo "  make play-graphics     watch trained agent in SDL2 window"
	@echo "  make hamiltonian       watch Hamiltonian-cycle agent (terminal)"
	@echo "  make hamiltonian-graphics  watch Hamiltonian-cycle agent (SDL2)"
	@echo "  make dqn-train         train DQN agent (DQN_EPISODES=5000 by default)"
	@echo "  make dqn-play          watch DQN agent in terminal"
	@echo "  make dqn-play-graphics watch DQN agent in SDL2 window"
	@echo "  make run               play the game yourself (terminal)"
	@echo "  make run-graphics      play the game yourself (SDL2)"
	@echo "  make clean             remove compiled objects"
	@echo "  make distclean         remove entire build directory"
	@echo ""
	@echo "Overrides:"
	@echo "  EPISODES=50000         number of Q-learning training episodes"
	@echo "  QTABLE=path/to/q.bin   Q-table file (default: build/qtable.bin)"
	@echo "  DQN_EPISODES=10000     number of DQN training episodes"
	@echo "  DQN_MODEL=path/to/m.pt DQN model file (default: build/dqn_model.pt)"
