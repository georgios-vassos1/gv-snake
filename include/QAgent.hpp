#ifndef QAGENT_H
#define QAGENT_H

#include "Game.hpp"
#include <string>

/// Tabular Q-learning agent for Snake.
///
/// State space (512 states):
///   bits [8:7]  current direction  w=0, s=1, a=2, d=3  →  4 values
///   bit  [6]    danger straight                          →  2 values
///   bit  [5]    danger left (relative)                  →  2 values
///   bit  [4]    danger right (relative)                 →  2 values
///   bit  [3]    food is above head  (fruit.x < head.x)  →  2 values
///   bit  [2]    food is below head  (fruit.x > head.x)  →  2 values
///   bit  [1]    food is left  of head (fruit.y < head.y) → 2 values
///   bit  [0]    food is right of head (fruit.y > head.y) → 2 values
///
///   Total: 4 × 2^7 = 512 states, 4 actions → 2 KiB Q-table.
///
/// "Dangerous" means the wrapped next cell in that direction holds a body
/// segment ('O').  The border wraps rather than kills, so '*' is ignored.
class QAgent {
public:
    static constexpr int   NUM_STATES   = 512;
    static constexpr int   NUM_ACTIONS  = 4;    ///< w, s, a, d
    static constexpr float ALPHA        = 0.1f; ///< learning rate
    static constexpr float GAMMA        = 0.9f; ///< discount factor
    static constexpr float EPSILON_MIN  = 0.01f;
    static constexpr float REWARD_FRUIT = 10.0f;
    static constexpr float REWARD_DEATH = -10.0f;
    static constexpr float REWARD_STEP  = -0.01f; ///< small step penalty

    /// Maps action index → direction character: {0='w', 1='s', 2='a', 3='d'}.
    static const char ACTIONS[NUM_ACTIONS];

    explicit QAgent(float epsilon = 1.0f);

    // ── State encoding ────────────────────────────────────────────────────────

    /// Encode the current game state as an integer in [0, NUM_STATES).
    static int encodeState(const Game& game);

    // ── Action selection ──────────────────────────────────────────────────────

    /// Epsilon-greedy: random with probability epsilon, greedy otherwise.
    int selectAction(int state) const;

    /// Pure greedy (argmax Q[state]); use this during evaluation.
    int greedyAction(int state) const;

    // ── Learning update ───────────────────────────────────────────────────────

    /// Standard Q-learning (off-policy TD) update:
    ///   Q[s][a] += alpha * (reward + gamma * max_a' Q[s'][a'] - Q[s][a])
    void update(int state, int action, float reward, int nextState);

    /// Terminal-state update (no successor state):
    ///   Q[s][a] += alpha * (reward - Q[s][a])
    void updateTerminal(int state, int action, float reward);

    /// Multiply epsilon by (1 - decayRate), clamped to EPSILON_MIN.
    void decayEpsilon(float decayRate);
    float getEpsilon() const { return epsilon_; }

    /// Compute the per-episode decay rate so that epsilon reaches EPSILON_MIN
    /// after exactly 'episodes' calls to decayEpsilon.
    static float computeDecayRate(int episodes);

    // ── Persistence ───────────────────────────────────────────────────────────

    /// Write Q-table to a binary file.  Returns false on I/O error.
    bool save(const std::string& path) const;

    /// Read Q-table from a binary file.  Returns false if file not found.
    bool load(const std::string& path);

    // ── Inspection (tests / debugging) ───────────────────────────────────────

    float getQ(int state, int action) const;

private:
    float Q_[NUM_STATES][NUM_ACTIONS];
    float epsilon_;

    /// Map direction character to 0-based index.
    static int  dirIndex(char dir);

    /// Given the current absolute direction and a relative offset
    /// (0 = straight, 1 = left, 2 = right), return the absolute direction.
    static char absoluteDir(char current, int relativeOffset);

    /// Compute the wrapped next cell when moving in 'dir' from (hx, hy).
    static void nextCell(int hx, int hy, char dir, int border,
                         int& nx, int& ny);

    /// Return true if the next cell in 'dir' holds a body segment.
    static bool isDangerous(const Game& game, char dir);
};

#endif // QAGENT_H
