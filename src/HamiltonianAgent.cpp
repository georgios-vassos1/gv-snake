#include "HamiltonianAgent.hpp"
#include "QAgent.hpp"
#include <cassert>
#include <cmath>

// ── Construction ──────────────────────────────────────────────────────────────

HamiltonianAgent::HamiltonianAgent(int border)
    : border_(border), interior_(border - 2), N_(interior_ * interior_)
{
    assert(interior_ > 0 && interior_ % 2 == 0 &&
           "HamiltonianAgent requires an even interior size");
    cycle_.reserve(static_cast<std::size_t>(N_));
    // order_ is 1-indexed: rows/cols 1..interior_
    const std::size_t dim = static_cast<std::size_t>(interior_) + 1;
    order_.assign(dim, std::vector<int>(dim, -1));
    buildCycle();
}

// ── Cycle construction ────────────────────────────────────────────────────────
//
// For an even n×n interior grid (1-indexed rows/cols 1..n) the cycle is:
//
//   1. Column 1 going DOWN:   (1,1) → (2,1) → … → (n,1)
//   2. Row n going RIGHT:     (n,2) → (n,3) → … → (n,n)
//   3. Zigzag upward in cols 2..n, rows n-1 down to 2:
//        row n-1 goes LEFT  (n,n)→(n-1,n)→…→(n-1,2)
//        row n-2 goes RIGHT (n-2,2)→…→(n-2,n)
//        … alternating …
//   4. Row 1 going LEFT:      (1,n) → (1,n-1) → … → (1,2)
//      wraps back to (1,1) = cycle[0]
//
// Every consecutive pair in the cycle is a direct grid neighbour (no border
// wrapping needed), so the snake never kills itself by following the cycle.

void HamiltonianAgent::buildCycle()
{
    const int n = interior_;

    // 1. Column 1 down.
    for (int r = 1; r <= n; ++r)
        cycle_.emplace_back(r, 1);

    // 2. Row n right.
    for (int c = 2; c <= n; ++c)
        cycle_.emplace_back(n, c);

    // 3. Zigzag upward: row n-1 → row 2.
    //    First zigzag row (n-1) goes LEFT; alternates each row.
    for (int r = n - 1; r >= 2; --r) {
        const bool goLeft = ((n - 1 - r) % 2 == 0);
        if (goLeft) {
            for (int c = n; c >= 2; --c)
                cycle_.emplace_back(r, c);
        } else {
            for (int c = 2; c <= n; ++c)
                cycle_.emplace_back(r, c);
        }
    }

    // 4. Row 1 left (wraps to cycle[0] = (1,1)).
    for (int c = n; c >= 2; --c)
        cycle_.emplace_back(1, c);

    assert(static_cast<int>(cycle_.size()) == N_);

    // Fill the inverse lookup (1-indexed).
    for (int i = 0; i < N_; ++i)
        order_[cycle_[static_cast<std::size_t>(i)].first]
              [cycle_[static_cast<std::size_t>(i)].second] = i;
}

// ── Helpers ───────────────────────────────────────────────────────────────────

char HamiltonianAgent::dirTo(int fromR, int fromC, int toR, int toC)
{
    if (toR < fromR) return 'w';
    if (toR > fromR) return 's';
    if (toC < fromC) return 'a';
    return 'd';
}

// ── Move selection ────────────────────────────────────────────────────────────

char HamiltonianAgent::nextMove(const Game& game) const
{
    const Point& head = game.getHead();
    const int    hr   = head.getX();
    const int    hc   = head.getY();
    const int    h    = order_[hr][hc];
    const int    f    = order_[game.getFruit().getX()][game.getFruit().getY()];

    // Default: follow the Hamiltonian cycle one step.
    // Normally safe, but shortcuts can leave a body segment at cycle[h+1] if the
    // snake lapped its own body; fall back to any flood-fill-safe direction in
    // that rare case.
    const int  nextIdx    = (h + 1) % N_;
    const int  nextR      = cycle_[static_cast<std::size_t>(nextIdx)].first;
    const int  nextC      = cycle_[static_cast<std::size_t>(nextIdx)].second;
    const char defaultDir = dirTo(hr, hc, nextR, nextC);

    // ── Shortcut logic ────────────────────────────────────────────────────────
    //
    // safeDelta: how many cycle steps ahead we can jump.
    //
    // Previous approaches (tail-delta, bodyLen bound, virtualFreeLen) all fail
    // when the body is fragmented by earlier shortcuts or when fruit growth places
    // the tail at the neck, making actualTailDelta ≈ N.
    //
    // Correct approach: scan forward in the cycle from h and find the nearest
    // occupied (body 'O') cell.  Any shortcut must land strictly before that cell
    // (and the cell right after the shortcut destination is also checked in the
    // loop below).  This is O(N-bodyLen) per call but N=324 makes it negligible.

    const int bodyLen        = game.getLength();
    const int virtualFreeLen = N_ - bodyLen;

    // Find nearest occupied cycle position ahead of h.
    int safeDelta = virtualFreeLen; // default: no body segment found within free zone
    for (int d = 1; d <= virtualFreeLen; ++d) {
        const int pos = (h + d) % N_;
        const auto& cell = cycle_[static_cast<std::size_t>(pos)];
        if (game.grid()[cell.first][cell.second] == 'O') {
            safeDelta = d;
            break;
        }
    }

    // When safeDelta == 1 the default cycle step (cycle[h+1]) is already
    // occupied — shortcuts are off the table.  Use a flood-fill safe fallback.
    if (safeDelta <= 1) {
        static const char DIRS4[4] = {'w', 's', 'a', 'd'};
        for (const char dir : DIRS4) {
            int nx = 0;
            int ny = 0;
            QAgent::nextCell(hr, hc, dir, border_, nx, ny);
            if (nx < 1 || nx > interior_ || ny < 1 || ny > interior_)
                continue;
            if (std::abs(nx - hr) + std::abs(ny - hc) != 1)
                continue;
            if (game.grid()[nx][ny] == 'O')
                continue;
            if (QAgent::floodFill(game, nx, ny) >= bodyLen)
                return dir;
        }
        return defaultDir; // fully trapped — accept fate
    }

    // Remaining cycle steps from h to food.
    const int defaultRemaining = (f - h + N_) % N_;

    char bestDir = 0;
    int  bestRem = defaultRemaining; // only take a shortcut if it strictly improves this

    static const char DIRS[4] = {'w', 's', 'a', 'd'};
    for (const char dir : DIRS) {
        int nx = 0;
        int ny = 0;
        QAgent::nextCell(hr, hc, dir, border_, nx, ny);

        // Must land in the interior via a non-wrapping step.
        // QAgent::nextCell uses toroidal wrapping, which would create body
        // junctions across the grid edge that corrupt the cycle-order invariant.
        if (nx < 1 || nx > interior_ || ny < 1 || ny > interior_)
            continue;
        if (std::abs(nx - hr) + std::abs(ny - hc) != 1)
            continue;
        // Must not be body.
        if (game.grid()[nx][ny] == 'O')
            continue;

        const int candPos = order_[nx][ny];
        const int delta   = (candPos - h + N_) % N_;

        // Must be strictly ahead and within the safe bound.
        if (delta == 0 || delta >= safeDelta)
            continue;
        // Require enough forward clearance left after the shortcut so the snake
        // doesn't end up boxed in.  safeDelta - delta is approximately the new
        // forward free zone; demand it stays above bodyLen/4.
        if (safeDelta - delta < bodyLen / 4)
            continue;
        // Must improve the remaining cycle distance to food.
        const int remDist = (f - candPos + N_) % N_;
        if (remDist >= bestRem)
            continue;
        // Flood-fill safety: require ample reachable space (2×bodyLen) so
        // consecutive shortcuts can't compress the snake into a dead end.
        if (QAgent::floodFill(game, nx, ny) < 2 * bodyLen)
            continue;

        bestRem = remDist;
        bestDir = dir;
    }

    return bestDir != 0 ? bestDir : defaultDir;
}
