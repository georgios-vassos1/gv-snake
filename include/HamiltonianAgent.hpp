#ifndef HAMILTONIAN_AGENT_H
#define HAMILTONIAN_AGENT_H

#include "Game.hpp"
#include <utility>
#include <vector>

/// Hamiltonian-cycle agent with greedy shortcuts.
///
/// Precomputes a Hamiltonian cycle over the interior cells of the grid.
/// At each step the agent follows the cycle by default.  If a shortcut
/// (a forward jump in cycle order toward the food) is available and passes
/// the BFS flood-fill safety check, it takes the shortcut that most reduces
/// the remaining cycle distance to food.
///
/// The cycle visits all N = interior² cells exactly once, so the snake can
/// never permanently miss the food.  Shortcuts make it reach food faster and
/// allow it to fill the board, achieving perfect or near-perfect scores.
///
/// Requires: (border - 2) must be even (satisfied for the default 20×20 grid).
class HamiltonianAgent {
public:
    /// border — full grid side length including border cells (e.g. 20).
    explicit HamiltonianAgent(int border);

    /// Returns the next direction character ('w','s','a','d').
    char nextMove(const Game& game) const;

    // ── Accessors (read-only, for testing) ────────────────────────────────────

    int                                     interior() const { return interior_; }
    int                                     cycleLen() const { return N_; }
    const std::vector<std::pair<int, int>>& cycle() const { return cycle_; }
    const std::vector<std::vector<int>>&    order() const { return order_; }

    /// Direction character from adjacent cell (fromR,fromC) to (toR,toC).
    static char dirTo(int fromR, int fromC, int toR, int toC);

private:
    int border_;
    int interior_; ///< border - 2
    int N_;        ///< interior_ * interior_  (total cycle length)

    /// cycle_[i] = {row, col}  (1-indexed game coordinates)
    std::vector<std::pair<int, int>> cycle_;

    /// order_[row][col] = position in cycle  (row/col are 1-indexed)
    std::vector<std::vector<int>> order_;

    void buildCycle();
};

#endif // HAMILTONIAN_AGENT_H
