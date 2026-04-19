#include "QAgent.hpp"
#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <queue>
#include <utility>
#include <vector>

// ── Static data ───────────────────────────────────────────────────────────────

const char QAgent::ACTIONS[NUM_ACTIONS] = {'w', 's', 'a', 'd'};

// ── Construction ──────────────────────────────────────────────────────────────

QAgent::QAgent(float epsilon) : epsilon_(epsilon)
{
    std::memset(Q_, 0, sizeof(Q_));
}

// ── Private helpers ───────────────────────────────────────────────────────────

int QAgent::dirIndex(char dir)
{
    switch (dir) {
    case 'w': return 0;
    case 's': return 1;
    case 'a': return 2;
    case 'd': return 3;
    default:
        assert(false && "dirIndex: invalid direction character");
        return 3; // unreachable; silences compiler warning
    }
}

/// Relative direction table.
/// Rows: current direction (w=0, s=1, a=2, d=3).
/// Cols: relative offset (0=straight, 1=left-turn, 2=right-turn).
///
/// Left/right are defined from the snake's perspective facing forward:
///   facing w (north): left → a (west), right → d (east)
///   facing s (south): left → d (east),  right → a (west)
///   facing a (west):  left → s (south), right → w (north)
///   facing d (east):  left → w (north), right → s (south)
char QAgent::absoluteDir(char current, int relativeOffset)
{
    static const char TABLE[4][3] = {
        {'w', 'a', 'd'}, // facing w
        {'s', 'd', 'a'}, // facing s
        {'a', 's', 'w'}, // facing a
        {'d', 'w', 's'}, // facing d
    };
    return TABLE[dirIndex(current)][relativeOffset];
}

/// Mirrors ShitList::nextHead exactly so danger detection stays consistent
/// with the game's own collision logic.
void QAgent::nextCell(int hx, int hy, char dir, int border, int& nx, int& ny)
{
    const int field = border - 2;
    switch (dir) {
    case 'w':
        nx = (hx - 2 + field) % field + 1;
        ny = hy;
        break;
    case 's':
        nx = hx % field + 1;
        ny = hy;
        break;
    case 'a':
        nx = hx;
        ny = (hy - 2 + field) % field + 1;
        break;
    case 'd':
        nx = hx;
        ny = hy % field + 1;
        break;
    default:
        nx = hx;
        ny = hy;
        break;
    }
}

bool QAgent::isDangerous(const Game& game, char dir)
{
    const Point& head = game.getHead();
    int          nx   = 0;
    int          ny   = 0;
    nextCell(head.getX(), head.getY(), dir, game.getBorder(), nx, ny);
    // Only 'O' (body segment) is fatal; the border wraps rather than kills.
    return game.grid()[nx][ny] == 'O';
}

// ── State encoding ────────────────────────────────────────────────────────────

int QAgent::encodeState(const Game& game)
{
    // If no move has been made yet, treat the direction as 'd' (rightward),
    // which matches the snake's initial horizontal orientation.
    const char   dir  = (game.getLastMove() != 0) ? game.getLastMove() : 'd';
    const Point& head = game.getHead();
    const Point& food = game.getFruit();

    const int dangerStraight = isDangerous(game, absoluteDir(dir, 0)) ? 1 : 0;
    const int dangerLeft     = isDangerous(game, absoluteDir(dir, 1)) ? 1 : 0;
    const int dangerRight    = isDangerous(game, absoluteDir(dir, 2)) ? 1 : 0;
    const int foodUp         = (food.getX() < head.getX()) ? 1 : 0;
    const int foodDown       = (food.getX() > head.getX()) ? 1 : 0;
    const int foodLeft       = (food.getY() < head.getY()) ? 1 : 0;
    const int foodRight      = (food.getY() > head.getY()) ? 1 : 0;

    const int idx = (dirIndex(dir) << 7) | (dangerStraight << 6) | (dangerLeft << 5) |
                    (dangerRight << 4) | (foodUp << 3) | (foodDown << 2) | (foodLeft << 1) |
                    (foodRight << 0);

    assert(idx >= 0 && idx < NUM_STATES);
    return idx;
}

// ── Safety filter ─────────────────────────────────────────────────────────────

int QAgent::floodFill(const Game& game, int startX, int startY)
{
    const int    border = game.getBorder();
    char* const* grid   = game.grid();

    if (grid[startX][startY] == 'O')
        return 0;

    std::vector<bool>               visited(static_cast<std::size_t>(border) * static_cast<std::size_t>(border), false);
    std::queue<std::pair<int, int>> q;

    visited[static_cast<std::size_t>(startX) * static_cast<std::size_t>(border) +
            static_cast<std::size_t>(startY)] = true;
    q.emplace(startX, startY);

    static const char DIRS[4] = {'w', 's', 'a', 'd'};
    int               count   = 0;
    while (!q.empty()) {
        const int x = q.front().first;
        const int y = q.front().second;
        q.pop();
        ++count;
        for (char dir : DIRS) {
            int nx = 0;
            int ny = 0;
            nextCell(x, y, dir, border, nx, ny);
            const std::size_t key = static_cast<std::size_t>(nx) * static_cast<std::size_t>(border) + static_cast<std::size_t>(ny);
            if (!visited[key] && grid[nx][ny] != 'O') {
                visited[key] = true;
                q.emplace(nx, ny);
            }
        }
    }
    return count;
}

int QAgent::safeAction(const Game& game, int state) const
{
    const int minCells = game.getLength();

    // Sort action indices by Q-value descending (insertion sort over 4 elements).
    int order[NUM_ACTIONS] = {0, 1, 2, 3};
    for (int i = 1; i < NUM_ACTIONS; ++i) {
        const int key = order[i];
        int       j   = i - 1;
        while (j >= 0 && Q_[state][order[j]] < Q_[state][key]) {
            order[j + 1] = order[j];
            --j;
        }
        order[j + 1] = key;
    }

    const Point& head   = game.getHead();
    const int    border = game.getBorder();

    for (int action : order) {
        const char dir    = ACTIONS[action];
        int        nx     = 0;
        int        ny     = 0;
        nextCell(head.getX(), head.getY(), dir, border, nx, ny);
        if (game.grid()[nx][ny] == 'O')
            continue; // immediately fatal
        if (floodFill(game, nx, ny) >= minCells)
            return action;
    }

    // Fully trapped — fall back to greedy to at least make a move.
    return greedyAction(state);
}

// ── Action selection ──────────────────────────────────────────────────────────

int QAgent::greedyAction(int state) const
{
    assert(state >= 0 && state < NUM_STATES);
    int best = 0;
    for (int a = 1; a < NUM_ACTIONS; ++a)
        if (Q_[state][a] > Q_[state][best])
            best = a;
    return best;
}

int QAgent::selectAction(int state) const
{
    assert(state >= 0 && state < NUM_STATES);
    if ((static_cast<float>(std::rand()) / static_cast<float>(RAND_MAX)) < epsilon_)
        return std::rand() % NUM_ACTIONS;
    return greedyAction(state);
}

// ── Learning update ───────────────────────────────────────────────────────────

void QAgent::update(int state, int action, float reward, int nextState)
{
    assert(state >= 0 && state < NUM_STATES);
    assert(action >= 0 && action < NUM_ACTIONS);
    assert(nextState >= 0 && nextState < NUM_STATES);
    const float maxNext = *std::max_element(Q_[nextState], Q_[nextState] + NUM_ACTIONS);
    Q_[state][action] += ALPHA * (reward + GAMMA * maxNext - Q_[state][action]);
}

void QAgent::updateTerminal(int state, int action, float reward)
{
    assert(state >= 0 && state < NUM_STATES);
    assert(action >= 0 && action < NUM_ACTIONS);
    Q_[state][action] += ALPHA * (reward - Q_[state][action]);
}

void QAgent::decayEpsilon(float decayRate)
{
    epsilon_ = std::max(EPSILON_MIN, epsilon_ * (1.0F - decayRate));
}

float QAgent::computeDecayRate(int episodes)
{
    assert(episodes > 0);
    return static_cast<float>(
        1.0 - std::pow(static_cast<double>(EPSILON_MIN), 1.0 / static_cast<double>(episodes)));
}

// ── Persistence ───────────────────────────────────────────────────────────────

bool QAgent::save(const std::string& path) const
{
    std::FILE* f = std::fopen(path.c_str(), "wb");
    if (!f)
        return false;
    const bool ok = (std::fwrite(Q_, sizeof(Q_), 1, f) == 1);
    std::fclose(f);
    return ok;
}

bool QAgent::load(const std::string& path)
{
    std::FILE* f = std::fopen(path.c_str(), "rb");
    if (!f)
        return false;
    const bool ok = (std::fread(Q_, sizeof(Q_), 1, f) == 1);
    std::fclose(f);
    return ok;
}

float QAgent::getQ(int state, int action) const
{
    assert(state >= 0 && state < NUM_STATES);
    assert(action >= 0 && action < NUM_ACTIONS);
    return Q_[state][action];
}
