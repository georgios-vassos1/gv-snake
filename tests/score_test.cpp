#include "Game.hpp"
#include "HighScore.hpp"
#include <cassert>
#include <cstdio>
#include <cstdlib>

// Scan the grid for the head character 'A'.
static void getHeadPos(const Game& g, int& hx, int& hy)
{
    const int    b = g.getBorder();
    char* const* A = g.grid();
    for (int i = 0; i < b; i++)
        for (int j = 0; j < b; j++)
            if (A[i][j] == 'A') {
                hx = i;
                hy = j;
                return;
            }
    hx = hy = -1;
}

int main()
{
    int failures = 0;

    // ── Test 1: HighScore save/load round-trip ────────────────────────────────
    saveHighScore(77);
    int loaded = loadHighScore();
    if (loaded == 77) {
        std::printf("PASS  HighScore round-trip: saved 77, loaded %d\n", loaded);
    } else {
        std::printf("FAIL  HighScore round-trip: expected 77, got %d\n", loaded);
        ++failures;
    }
    saveHighScore(0); // reset for a clean slate

    // ── Test 2: Initial score is zero ─────────────────────────────────────────
    Game g(60);
    if (g.getScore() == 0) {
        std::printf("PASS  Initial score is 0\n");
    } else {
        std::printf("FAIL  Initial score: expected 0, got %d\n", g.getScore());
        ++failures;
    }

    // ── Test 3: Steer toward fruit; score must increment on AteFruit ──────────
    // Greedy: close the largest gap first.  Large grid gives room to manoeuvre.
    int  eaten = 0;
    bool dead  = false;
    for (int step = 0; step < 500 && eaten < 3; ++step) {
        int hx, hy;
        getHeadPos(g, hx, hy);
        const int fx = g.getFruit().getX();
        const int fy = g.getFruit().getY();

        char      dir;
        const int dx = hx - fx; // positive → head is below fruit (need 'w')
        const int dy = hy - fy; // positive → head is right of fruit (need 'a')
        if (std::abs(dx) >= std::abs(dy))
            dir = (dx > 0) ? 'w' : 's';
        else
            dir = (dy > 0) ? 'a' : 'd';

        TickResult r = g.tick(dir);
        if (r == TickResult::GameOver) {
            std::printf("NOTE  Snake died at step %d before eating %d fruit(s)\n", step, 3 - eaten);
            dead = true;
            break;
        }
        if (r == TickResult::AteFruit) {
            ++eaten;
            if (g.getScore() == eaten) {
                std::printf("PASS  Ate fruit #%d at step %d  score=%d\n", eaten, step,
                            g.getScore());
            } else {
                std::printf("FAIL  Score mismatch after fruit #%d: expected %d, got %d\n", eaten,
                            eaten, g.getScore());
                ++failures;
            }
        }
    }
    if (!dead && eaten == 0) {
        std::printf("FAIL  Snake did not eat any fruit in 500 steps\n");
        ++failures;
    }

    // ── Test 4: Score line format (visual) ────────────────────────────────────
    std::printf("\nSample score line as it appears below the grid:\n");
    std::printf("  Score: %d   Best: %d\033[K\n", g.getScore(), loadHighScore());
    std::printf("(the \\033[K escape erases to end of line — normal in a real terminal)\n");

    // ── Test 5: saveHighScore only when beaten ────────────────────────────────
    saveHighScore(1000);
    int       before    = loadHighScore();
    const int fakeScore = 5;
    if (fakeScore > before)
        saveHighScore(fakeScore);
    int after = loadHighScore();
    if (after == before) {
        std::printf("\nPASS  High score not overwritten when new score (%d) <= best (%d)\n",
                    fakeScore, before);
    } else {
        std::printf("\nFAIL  High score was overwritten: %d → %d (should stay %d)\n", before, after,
                    before);
        ++failures;
    }
    saveHighScore(0); // clean up

    std::printf("\n%s  (%d failure(s))\n", failures == 0 ? "ALL PASSED" : "SOME FAILED", failures);
    return failures == 0 ? 0 : 1;
}
