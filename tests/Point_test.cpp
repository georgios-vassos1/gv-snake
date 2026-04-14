#include "Point.hpp"
#include <cstdio>

static int failures = 0;

#define EXPECT(cond, msg)                                                          \
    do {                                                                           \
        if (cond) {                                                                \
            std::printf("PASS  %s\n", msg);                                        \
        } else {                                                                   \
            std::printf("FAIL  %s\n", msg);                                        \
            ++failures;                                                            \
        }                                                                          \
    } while (0)

int main()
{
    // ── moveUp ────────────────────────────────────────────────────────────────
    // x is the row index; "up" means toward row 0, i.e. decrements x.
    {
        Point p(5, 3);
        p.moveUp();
        EXPECT(p.getX() == 4, "moveUp: x decremented by 1");
        EXPECT(p.getY() == 3, "moveUp: y unchanged");
    }

    // ── moveDown ──────────────────────────────────────────────────────────────
    {
        Point p(5, 3);
        p.moveDown();
        EXPECT(p.getX() == 6, "moveDown: x incremented by 1");
        EXPECT(p.getY() == 3, "moveDown: y unchanged");
    }

    // ── moveLeft ──────────────────────────────────────────────────────────────
    {
        Point p(5, 3);
        p.moveLeft();
        EXPECT(p.getX() == 5, "moveLeft: x unchanged");
        EXPECT(p.getY() == 2, "moveLeft: y decremented by 1");
    }

    // ── moveRight ─────────────────────────────────────────────────────────────
    {
        Point p(5, 3);
        p.moveRight();
        EXPECT(p.getX() == 5, "moveRight: x unchanged");
        EXPECT(p.getY() == 4, "moveRight: y incremented by 1");
    }

    // ── Axes are independent over multiple steps ───────────────────────────────
    // Ten consecutive vertical moves must not disturb the y coordinate.
    {
        Point p(0, 7);
        for (int i = 0; i < 10; ++i)
            p.moveDown();
        EXPECT(p.getX() == 10, "10x moveDown: x accumulates correctly");
        EXPECT(p.getY() == 7,  "10x moveDown: y stays fixed");
    }
    // Ten consecutive horizontal moves must not disturb the x coordinate.
    {
        Point p(4, 0);
        for (int i = 0; i < 10; ++i)
            p.moveRight();
        EXPECT(p.getX() == 4,  "10x moveRight: x stays fixed");
        EXPECT(p.getY() == 10, "10x moveRight: y accumulates correctly");
    }

    std::printf("\n%s  (%d failure(s))\n", failures == 0 ? "ALL PASSED" : "SOME FAILED", failures);
    return failures == 0 ? 0 : 1;
}
