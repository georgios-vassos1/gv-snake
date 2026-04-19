#include "Game.hpp"
#include "HamiltonianAgent.hpp"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <set>

// ── Tiny test harness ─────────────────────────────────────────────────────────

static int failures = 0;

#define PASS(msg) std::printf("PASS  %s\n", msg)
#define FAIL(msg, ...)                                                                             \
    do {                                                                                           \
        std::printf("FAIL  " msg "\n", ##__VA_ARGS__);                                             \
        ++failures;                                                                                \
    } while (0)

// ── 1. Cycle has exactly N = interior² entries ────────────────────────────────
//
// If buildCycle emits too few or too many cells the cycle is not Hamiltonian.

static void testCycleLength()
{
    HamiltonianAgent agent(20);
    const int        n = agent.interior();
    const int        N = n * n; // 324

    if (agent.cycleLen() != N)
        FAIL("cycleLen() = %d, expected %d", agent.cycleLen(), N);
    else if (static_cast<int>(agent.cycle().size()) != N)
        FAIL("cycle().size() = %d, expected %d", static_cast<int>(agent.cycle().size()), N);
    else
        PASS("cycle length == interior²");
}

// ── 2. Every cell in the cycle is inside the interior [1..n]² ─────────────────
//
// A cell outside the interior would land on the border or out of bounds.

static void testCycleBounds()
{
    HamiltonianAgent agent(20);
    const int        n = agent.interior();

    for (int i = 0; i < agent.cycleLen(); ++i) {
        const int r = agent.cycle()[i].first;
        const int c = agent.cycle()[i].second;
        if (r < 1 || r > n || c < 1 || c > n) {
            FAIL("cycle[%d] = (%d,%d) is outside interior [1..%d]", i, r, c, n);
            return;
        }
    }
    PASS("all cycle cells are within interior bounds");
}

// ── 3. Cycle visits every interior cell exactly once ──────────────────────────
//
// Duplicate or missing cells mean the cycle is not a permutation of the grid.

static void testCycleCoversAllCells()
{
    HamiltonianAgent agent(20);
    const int        n = agent.interior();

    std::set<std::pair<int, int>> visited;
    for (int i = 0; i < agent.cycleLen(); ++i) {
        if (!visited.insert(agent.cycle()[i]).second) {
            const int r = agent.cycle()[i].first;
            const int c = agent.cycle()[i].second;
            FAIL("cycle[%d] = (%d,%d) is a duplicate", i, r, c);
            return;
        }
    }

    if (static_cast<int>(visited.size()) != n * n)
        FAIL("cycle visits %d cells, expected %d", static_cast<int>(visited.size()), n * n);
    else
        PASS("cycle visits every interior cell exactly once");
}

// ── 4. Every consecutive pair is Manhattan-distance-1 (no wrapping) ───────────
//
// This is the property that guarantees following the cycle never causes a
// toroidal jump.  The bug that killed the agent in early development was
// exactly a wrapped shortcut creating distance > 1.

static void testCycleAdjacency()
{
    HamiltonianAgent agent(20);
    const int        N = agent.cycleLen();

    for (int i = 0; i < N; ++i) {
        const int nextIdx = (i + 1) % N;
        const int dr      = agent.cycle()[nextIdx].first - agent.cycle()[i].first;
        const int dc      = agent.cycle()[nextIdx].second - agent.cycle()[i].second;
        const int dist    = std::abs(dr) + std::abs(dc);
        if (dist != 1) {
            FAIL("cycle[%d]→cycle[%d]: (%d,%d)→(%d,%d) distance=%d, expected 1", i, nextIdx,
                 agent.cycle()[i].first, agent.cycle()[i].second, agent.cycle()[nextIdx].first,
                 agent.cycle()[nextIdx].second, dist);
            return;
        }
    }
    PASS("every consecutive cycle pair is Manhattan-distance-1");
}

// ── 5. order_ is the exact inverse of cycle_ ─────────────────────────────────
//
// order[r][c] must equal i iff cycle[i] == (r,c).  If this invariant breaks,
// the shortcut delta computation produces wrong values.

static void testOrderInverse()
{
    HamiltonianAgent agent(20);
    const int        N = agent.cycleLen();

    for (int i = 0; i < N; ++i) {
        const int r   = agent.cycle()[i].first;
        const int c   = agent.cycle()[i].second;
        const int ord = agent.order()[r][c];
        if (ord != i) {
            FAIL("order[%d][%d] = %d, expected %d (cycle[%d])", r, c, ord, i, i);
            return;
        }
    }
    PASS("order[][] is the exact inverse of cycle[]");
}

// ── 6. dirTo returns correct direction for all four cardinal moves ────────────
//
// dirTo is used to convert a (from, to) cell pair into a movement character.
// A wrong mapping means the snake moves the opposite direction from intended.

static void testDirTo()
{
    // up: row decreases
    if (HamiltonianAgent::dirTo(5, 3, 4, 3) != 'w') {
        FAIL("dirTo(5,3 → 4,3) should be 'w'");
        return;
    }
    // down: row increases
    if (HamiltonianAgent::dirTo(5, 3, 6, 3) != 's') {
        FAIL("dirTo(5,3 → 6,3) should be 's'");
        return;
    }
    // left: col decreases
    if (HamiltonianAgent::dirTo(5, 3, 5, 2) != 'a') {
        FAIL("dirTo(5,3 → 5,2) should be 'a'");
        return;
    }
    // right: col increases
    if (HamiltonianAgent::dirTo(5, 3, 5, 4) != 'd') {
        FAIL("dirTo(5,3 → 5,4) should be 'd'");
        return;
    }
    PASS("dirTo returns correct direction for all four cardinal moves");
}

// ── 7. Agent achieves perfect score across multiple random seeds ──────────────
//
// This is the end-to-end property: shortcuts + safety logic must never cause
// death and must always collect all 314 fruits.  If any of the above invariants
// were silently broken (e.g. wrong order_ but right cycle_), this test would
// catch the downstream failure.

static void testPerfectScore()
{
    const int border   = 20;
    const int interior = border - 2;
    const int maxScore = interior * interior - 10; // 314
    const int maxIdle  = border * border * 2;
    const int episodes = 5;

    HamiltonianAgent agent(border);

    for (int ep = 0; ep < episodes; ++ep) {
        Game game(border);
        int  idle = 0;
        while (true) {
            const char       dir    = agent.nextMove(game);
            const TickResult result = game.tick(dir);
            if (result == TickResult::GameOver) {
                FAIL("agent died in episode %d at score %d", ep + 1, game.getScore());
                return;
            }
            if (result == TickResult::AteFruit) {
                idle = 0;
            } else {
                ++idle;
            }
            if (idle >= maxIdle) {
                FAIL("agent stalled in episode %d at score %d (idle limit reached)", ep + 1,
                     game.getScore());
                return;
            }
            if (game.getScore() >= maxScore)
                break;
        }
        if (game.getScore() != maxScore) {
            FAIL("episode %d: score %d, expected %d", ep + 1, game.getScore(), maxScore);
            return;
        }
    }
    PASS("perfect score (314) in all 5 episodes");
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main()
{
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    testCycleLength();
    testCycleBounds();
    testCycleCoversAllCells();
    testCycleAdjacency();
    testOrderInverse();
    testDirTo();
    testPerfectScore();

    std::printf("\n%s (%d failure%s)\n", failures == 0 ? "ALL PASSED" : "SOME FAILED", failures,
                failures == 1 ? "" : "s");
    return failures == 0 ? 0 : 1;
}
