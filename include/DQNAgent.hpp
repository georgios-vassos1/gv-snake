#ifndef DQN_AGENT_H
#define DQN_AGENT_H

#ifdef DQN_AVAILABLE

#include "Game.hpp"
#include <deque>
#include <random>
#include <string>
#include <torch/torch.h>

// ── Network ──────────────────────────────────────────────────────────────────

/// Compact-feature MLP: 20 → 64 → 32 → 4.
struct DQNNetImpl : torch::nn::Module {
    static constexpr int INPUT_SIZE = 20;

    torch::nn::Linear fc1{nullptr}, fc2{nullptr}, fc3{nullptr};

    DQNNetImpl();
    torch::Tensor forward(torch::Tensor x);
};
TORCH_MODULE(DQNNet);

// ── Replay buffer entry ──────────────────────────────────────────────────────

struct Transition {
    torch::Tensor state;
    int           action;
    float         reward;
    torch::Tensor nextState;
    bool          done;
};

// ── Agent ────────────────────────────────────────────────────────────────────

class DQNAgent {
public:
    static constexpr int   NUM_ACTIONS      = 4;
    static constexpr int   REPLAY_CAPACITY  = 50000;
    static constexpr int   BATCH_SIZE       = 64;
    static constexpr int   TARGET_UPDATE    = 1000;  ///< steps between target-net sync
    static constexpr int   MIN_REPLAY       = 1000;  ///< don't train until this many transitions
    static constexpr int   TRAIN_EVERY      = 4;     ///< train once per N env steps
    static constexpr float GAMMA            = 0.99F;
    static constexpr float LR               = 1e-3F;
    static constexpr float EPSILON_START    = 1.0F;
    static constexpr float EPSILON_END      = 0.01F;

    DQNAgent();

    /// Encode game state as a compact [1, 20] feature tensor.
    ///
    /// Features (all normalised to roughly [-1, 1]):
    ///   [0-3]   danger 1 step in each absolute direction (w/s/a/d)   binary
    ///   [4-7]   danger 2 steps in each absolute direction            binary
    ///   [8-11]  current direction one-hot (w/s/a/d)
    ///   [12-13] normalised food delta-x and delta-y  (head-relative)
    ///   [14]    normalised Manhattan distance to food
    ///   [15]    normalised snake body length
    ///   [16-19] flood-fill safety ratio in each direction (0=fatal, 1=fully open)
    static torch::Tensor encodeState(const Game& game);

    /// Epsilon-greedy (training) or pure greedy (play).
    int selectAction(const torch::Tensor& state, bool training = true);

    /// Pure greedy for play mode.
    int greedyAction(const torch::Tensor& state) const;

    /// Store transition and run a training step if buffer is large enough.
    void step(const torch::Tensor& state, int action, float reward,
              const torch::Tensor& nextState, bool done);

    /// Save / load the policy network.
    bool save(const std::string& path) const;
    bool load(const std::string& path);

    /// Multiply epsilon by (1 - decayRate), clamped to EPSILON_END.
    void decayEpsilon(float decayRate);

    /// Compute per-episode decay rate so epsilon reaches EPSILON_END after N episodes.
    static float computeDecayRate(int episodes);

    float getEpsilon() const { return epsilon_; }
    int   getStepCount() const { return stepCount_; }

private:
    mutable DQNNet          policyNet_;
    DQNNet                  targetNet_;
    torch::optim::Adam      optimizer_;
    std::deque<Transition>  replayBuffer_;
    std::mt19937            rng_;
    int                     stepCount_;
    float                   epsilon_;

    void trainBatch();
    void updateTargetNetwork();
};

#endif // DQN_AVAILABLE
#endif // DQN_AGENT_H
