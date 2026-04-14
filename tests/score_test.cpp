#include "Game.hpp"
#include "HighScore.hpp"
#include <cstdio>
#include <cstdlib>

static int failures = 0;

#define PASS(msg)         std::printf("PASS  %s\n", msg)
#define FAIL(msg, ...)                                                             \
    do {                                                                           \
        std::printf("FAIL  " msg "\n", ##__VA_ARGS__);                             \
        ++failures;                                                                \
    } while (0)
#define EXPECT(cond, msg) do { if (cond) PASS(msg); else FAIL("%s", msg); } while (0)

static void getHeadPos(const Game& g, int& hx, int& hy)
{
    const int    b = g.getBorder();
    char* const* A = g.grid();
    for (int i = 0; i < b; i++)
        for (int j = 0; j < b; j++)
            if (A[i][j] == 'A') { hx = i; hy = j; return; }
    hx = hy = -1;
}

int main()
{
    // ── Test 1: HighScore round-trip ──────────────────────────────────────────
    // saveHighScore followed by loadHighScore must return the same value.
    std::printf("\n-- Test 1: HighScore round-trip --\n");
    {
        saveHighScore(42);
        const int loaded = loadHighScore();
        if (loaded == 42)
            PASS("HighScore round-trip: saved 42, loaded 42");
        else
            FAIL("HighScore round-trip: expected 42, got %d", loaded);
        saveHighScore(0);
    }

    // ── Test 2: saveHighScore unconditionally overwrites ──────────────────────
    // The function has no guard logic — it always writes whatever value it
    // receives.  (The caller is responsible for deciding whether to save.)
    // If this test fails it means saveHighScore silently discarded the write.
    std::printf("\n-- Test 2: saveHighScore is unconditional --\n");
    {
        saveHighScore(1000);
        saveHighScore(3);               // lower value — must still be stored
        const int after = loadHighScore();
        if (after == 3)
            PASS("saveHighScore(3) overwrites previously saved 1000");
        else
            FAIL("saveHighScore(3) did not overwrite: got %d (expected 3)", after);
        saveHighScore(0);
    }

    // ── Test 3: Initial game score is zero ────────────────────────────────────
    std::printf("\n-- Test 3: initial score --\n");
    {
        Game g(60);
        EXPECT(g.getScore() == 0, "fresh game: score is 0");
    }

    // ── Test 4: score increments by exactly 1 on AteFruit, unchanged otherwise─
    // Steer greedily toward the fruit.  For every tick:
    //   - Running  → score must not have changed
    //   - AteFruit → score must have incremented by exactly 1
    std::printf("\n-- Test 4: score tracks AteFruit count --\n");
    {
        Game g(60);
        int  expectedScore = 0;
        int  prevScore     = 0;
        bool inconsistent  = false;

        for (int step = 0; step < 500 && expectedScore < 3; ++step) {
            int hx = 0;
            int hy = 0;
            getHeadPos(g, hx, hy);
            const int fx  = g.getFruit().getX();
            const int fy  = g.getFruit().getY();
            const int dx  = hx - fx;
            const int dy  = hy - fy;
            char      dir;
            if (std::abs(dx) >= std::abs(dy))
                dir = (dx > 0) ? 'w' : 's';
            else
                dir = (dy > 0) ? 'a' : 'd';

            prevScore      = g.getScore();
            TickResult r   = g.tick(dir);

            if (r == TickResult::GameOver)
                break;

            if (r == TickResult::Running && g.getScore() != prevScore) {
                FAIL("score changed on Running tick: %d → %d", prevScore, g.getScore());
                inconsistent = true;
                break;
            }
            if (r == TickResult::AteFruit) {
                ++expectedScore;
                if (g.getScore() != expectedScore) {
                    FAIL("score after fruit #%d: expected %d, got %d",
                         expectedScore, expectedScore, g.getScore());
                    inconsistent = true;
                    break;
                }
            }
        }
        if (!inconsistent) {
            if (expectedScore > 0)
                std::printf("PASS  score tracks AteFruit count (ate %d fruit(s))\n", expectedScore);
            else
                FAIL("snake died before eating any fruit — AteFruit path never exercised");
        }
    }

    std::printf("\n%s  (%d failure(s))\n", failures == 0 ? "ALL PASSED" : "SOME FAILED", failures);
    return failures == 0 ? 0 : 1;
}
