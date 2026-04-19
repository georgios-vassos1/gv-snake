#ifndef QAGENT_H
#define QAGENT_H

#include "Game.hpp"
#include <string>

/// Tabular Q-learning agent for Snake.
///
/// State space (32768 states):
///   bits [14:13]  current direction  w=0, s=1, a=2, d=3         →  4 values
///   bits [12:10]  danger straight 1/2/3 steps ahead              →  8 values
///   bits  [9: 7]  danger left    1/2/3 steps ahead               →  8 values
///   bits  [6: 4]  danger right   1/2/3 steps ahead               →  8 values
///   bits  [3: 2]  food X bin  0=far-above  1=near-above-or-same  →  4 values
///                             2=near-below 3=far-below
///   bits  [1: 0]  food Y bin  0=far-left   1=near-left-or-same   →  4 values
///                             2=near-right 3=far-right
///   (near = within interior/4 cells; far = beyond interior/4 cells)
///
///   Total: 4 × 2^13 = 32768 states, 4 actions → 512 KiB Q-table.
///
/// "Dangerous" means a cell N steps ahead in that direction holds a body
/// segment ('O').  The border wraps rather than kills, so '*' is ignored.
class QAgent {
public:
    static constexpr int   NUM_STATES   = 32768;
    static constexpr int   NUM_ACTIONS  = 4;    ///< w, s, a, d
    static constexpr float ALPHA        = 0.1F; ///< learning rate
    static constexpr float GAMMA        = 0.9F; ///< discount factor
    static constexpr float EPSILON_MIN  = 0.01F;
    static constexpr float REWARD_FRUIT = 10.0F;
    static constexpr float REWARD_DEATH = -10.0F;
    static constexpr float REWARD_STEP  = -0.01F; ///< small step penalty

    /// Maps action index → direction character: {0='w', 1='s', 2='a', 3='d'}.
    static const char ACTIONS[NUM_ACTIONS];

    explicit QAgent(float epsilon = 1.0F);

    // ── State encoding ────────────────────────────────────────────────────────

    /// Encode the current game state as an integer in [0, NUM_STATES).
    static int encodeState(const Game& game);

    // ── Food distance binning ─────────────────────────────────────────────────

    /// Bin a signed axis delta into [0, 3]:
    ///   0 = far negative  (delta < -thresh)
    ///   1 = near negative or same  (-thresh <= delta <= 0)
    ///   2 = near positive  (0 < delta <= thresh)
    ///   3 = far positive   (delta > thresh)
    /// thresh = max(1, interior/4) so the threshold scales with board size.
    static int foodDistBin(int delta, int interior);

    // ── Safety filter ─────────────────────────────────────────────────────────

    /// BFS flood-fill from (startX, startY) using wrapping movement.
    /// Returns the number of interior cells reachable without crossing a body
    /// segment ('O').  The start cell itself is counted if it is not 'O'.
    static int floodFill(const Game& game, int startX, int startY);

    /// Like greedyAction but filters out moves that leave the snake's reachable
    /// area smaller than its body length.  Actions are tried in descending
    /// Q-value order; the first one that passes the flood-fill check is returned.
    /// Falls back to greedyAction when every move is unsafe (fully trapped).
    int safeAction(const Game& game, int state) const;

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
    void  decayEpsilon(float decayRate);
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
    static int dirIndex(char dir);

    /// Given the current absolute direction and a relative offset
    /// (0 = straight, 1 = left, 2 = right), return the absolute direction.
    static char absoluteDir(char current, int relativeOffset);

    /// Compute the wrapped next cell when moving in 'dir' from (hx, hy).
    static void nextCell(int hx, int hy, char dir, int border, int& nx, int& ny);

    /// Return true if the cell exactly 'steps' steps ahead in 'absDir' holds a
    /// body segment ('O').  Each step uses the same absolute direction (straight
    /// line), consistent with the wrapping movement model.
    static bool isDangerousN(const Game& game, char absDir, int steps);
};

#endif // QAGENT_H
