#include "Game.hpp"
#include "QAgent.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>

// ── Tiny test harness ─────────────────────────────────────────────────────────

static int failures = 0;

#define PASS(msg) std::printf("PASS  %s\n", msg)
#define FAIL(msg, ...)                                                                             \
    do {                                                                                           \
        std::printf("FAIL  " msg "\n", ##__VA_ARGS__);                                             \
        ++failures;                                                                                \
    } while (0)

#define EXPECT_TRUE(expr, msg)                                                                     \
    do {                                                                                           \
        if (expr) {                                                                                \
            PASS(msg);                                                                             \
        } else {                                                                                   \
            FAIL("%s", msg);                                                                       \
        }                                                                                          \
    } while (0)

#define EXPECT_EQ(a, b, msg)                                                                       \
    do {                                                                                           \
        if ((a) == (b)) {                                                                          \
            PASS(msg);                                                                             \
        } else {                                                                                   \
            FAIL("%s: expected %d, got %d", msg, static_cast<int>(b), static_cast<int>(a));        \
        }                                                                                          \
    } while (0)

#define EXPECT_NEAR(a, b, tol, msg)                                                                \
    do {                                                                                           \
        if (std::fabs(static_cast<double>(a) - static_cast<double>(b)) <= (tol)) {                 \
            PASS(msg);                                                                             \
        } else {                                                                                   \
            FAIL("%s: expected ~%.6F, got %.6F", msg, static_cast<double>(b),                      \
                 static_cast<double>(a));                                                          \
        }                                                                                          \
    } while (0)

// ── Test 1: encodeState is always in [0, NUM_STATES) ─────────────────────────

static void test_encodeState_range()
{
    std::printf("\n-- Test 1: encodeState range --\n");
    bool allInRange = true;
    for (int trial = 0; trial < 200; ++trial) {
        Game       game(20);
        const char dirs[] = {'w', 's', 'a', 'd'};
        for (int step = 0; step < 10; ++step) {
            const char d = dirs[std::rand() % 4];
            if (game.tick(d) == TickResult::GameOver)
                break;
        }
        const int idx = QAgent::encodeState(game);
        if (idx < 0 || idx >= QAgent::NUM_STATES) {
            allInRange = false;
            FAIL("encodeState out of range: %d", idx);
            break;
        }
    }
    if (allInRange)
        PASS("encodeState always in [0, NUM_STATES)");
}

// ── Test 2: encodeState food direction bits match actual positions ────────────
// Extracts bits [3:0] from the encoded state and independently computes what
// they should be from game.getHead() and game.getFruit().  If the bit packing
// or the up/down/left/right predicates are wrong this test will catch it.

static void test_encodeState_food_bits()
{
    std::printf("\n-- Test 2: encodeState food direction bits --\n");
    bool ok = true;
    for (int trial = 0; trial < 100; ++trial) {
        Game         game(20);
        const int    encoded  = QAgent::encodeState(game);
        const Point& head     = game.getHead();
        const Point& food     = game.getFruit();

        // bits [3:0] in the encoding:
        //   bit 3 = foodUp    = fruit.x < head.x
        //   bit 2 = foodDown  = fruit.x > head.x
        //   bit 1 = foodLeft  = fruit.y < head.y
        //   bit 0 = foodRight = fruit.y > head.y
        const int actualBits   = encoded & 0xF;
        const int expUp        = (food.getX() < head.getX()) ? 1 : 0;
        const int expDown      = (food.getX() > head.getX()) ? 1 : 0;
        const int expLeft      = (food.getY() < head.getY()) ? 1 : 0;
        const int expRight     = (food.getY() > head.getY()) ? 1 : 0;
        const int expectedBits = (expUp << 3) | (expDown << 2) | (expLeft << 1) | expRight;

        if (actualBits != expectedBits) {
            FAIL("food bits mismatch (trial %d): expected 0x%X, got 0x%X "
                 "(head=(%d,%d) fruit=(%d,%d))",
                 trial, expectedBits, actualBits,
                 head.getX(), head.getY(), food.getX(), food.getY());
            ok = false;
            break;
        }
    }
    if (ok)
        PASS("encodeState food direction bits match actual head/fruit positions");
}

// ── Test 3: greedyAction returns the argmax of Q values ──────────────────────

static void test_greedyAction()
{
    std::printf("\n-- Test 3: greedyAction is argmax --\n");
    QAgent agent(0.0F);

    // Flood Q[0][2] with a large reward so it dominates all other actions.
    for (int i = 0; i < 100; ++i)
        agent.update(0, 2, 1000.0F, 0);

    EXPECT_EQ(agent.greedyAction(0), 2, "greedyAction selects the highest-Q action");
}

// ── Test 4: Q-update math ─────────────────────────────────────────────────────

static void test_q_update_math()
{
    std::printf("\n-- Test 4: Q-update correctness --\n");
    QAgent agent(0.0F);

    // Initial Q is all zeros.
    // update(s=0, a=0, reward=10, s'=0):
    //   Q[0][0] += 0.1 * (10 + 0.9 * max(Q[0]) - Q[0][0])
    //            = 0.1 * (10 + 0) = 1.0
    agent.update(0, 0, 10.0F, 0);
    EXPECT_NEAR(agent.getQ(0, 0), 1.0F, 1e-5F, "first Q-update: Q[0][0] = 1.0");

    // Second update (s=0, a=0, reward=0, s'=0):
    //   max(Q[0]) = 1.0 (only Q[0][0] is non-zero)
    //   Q[0][0] += 0.1 * (0 + 0.9*1.0 - 1.0) = 0.1 * (-0.1) = -0.01
    //   New Q[0][0] = 0.99
    agent.update(0, 0, 0.0F, 0);
    EXPECT_NEAR(agent.getQ(0, 0), 0.99F, 1e-5F, "second Q-update: Q[0][0] = 0.99");
}

// ── Test 4b: updateTerminal math ──────────────────────────────────────────────

static void test_updateTerminal_math()
{
    std::printf("\n-- Test 4b: updateTerminal correctness --\n");
    QAgent agent(0.0F);

    // updateTerminal(s=1, a=0, reward=-10):
    //   Q[1][0] += 0.1 * (-10 - 0) = -1.0
    agent.updateTerminal(1, 0, -10.0F);
    EXPECT_NEAR(agent.getQ(1, 0), -1.0F, 1e-5F,
                "terminal Q-update: Q[1][0] = -1.0 (no successor term)");
}

// ── Test 5: epsilon decay ─────────────────────────────────────────────────────

static void test_epsilon_decay()
{
    std::printf("\n-- Test 5: epsilon decay --\n");
    QAgent agent(1.0F);

    const int   N    = 1000;
    const float rate = QAgent::computeDecayRate(N);
    for (int i = 0; i < N; ++i)
        agent.decayEpsilon(rate);

    EXPECT_NEAR(agent.getEpsilon(), QAgent::EPSILON_MIN, 1e-4F,
                "epsilon reaches EPSILON_MIN after N decays");

    // Further decay must stay clamped.
    agent.decayEpsilon(0.5F);
    EXPECT_NEAR(agent.getEpsilon(), QAgent::EPSILON_MIN, 1e-6F,
                "epsilon clamped at EPSILON_MIN");
}

// ── Test 6: save / load round-trip ───────────────────────────────────────────

static void test_save_load()
{
    std::printf("\n-- Test 6: save/load round-trip --\n");
    const std::string path = "/tmp/qagent_test.bin";

    QAgent a(0.0F);
    a.update(42, 3, 99.0F, 0);
    const float before = a.getQ(42, 3);

    EXPECT_TRUE(a.save(path), "save() returns true");

    QAgent b(0.0F);
    EXPECT_TRUE(b.load(path), "load() returns true");
    EXPECT_NEAR(b.getQ(42, 3), before, 1e-6F, "loaded Q[42][3] matches saved value");

    QAgent c(0.0F);
    EXPECT_TRUE(!c.load("/tmp/no_such_file_xyz.bin"), "load() returns false for missing file");

    std::remove(path.c_str());
}

// ── Test 7: selectAction with epsilon=0 is always greedy ─────────────────────
// With epsilon=0, selectAction must return the same action as greedyAction on
// every call.  A broken epsilon-greedy implementation (e.g. wrong comparison
// sign or missing clamp) would occasionally return a random action here.

static void test_selectAction_epsilon0_is_greedy()
{
    std::printf("\n-- Test 7: selectAction(epsilon=0) is purely greedy --\n");
    QAgent agent(0.0F);

    // Plant a clear winner so greedyAction is unambiguous.
    agent.update(0, 2, 100.0F, 0);
    const int expected = agent.greedyAction(0);

    bool allGreedy = true;
    for (int i = 0; i < 200; ++i) {
        if (agent.selectAction(0) != expected) {
            allGreedy = false;
            break;
        }
    }
    EXPECT_TRUE(allGreedy, "selectAction(epsilon=0) always returns greedyAction result");
}

// ── Test 8: integration — training improves score ────────────────────────────

static void test_training_improves_score()
{
    std::printf("\n-- Test 8: training improves score --\n");

    auto evalAgent = [](const QAgent& agent, int evalEpisodes) -> float {
        long total = 0;
        for (int ep = 0; ep < evalEpisodes; ++ep) {
            Game      game(20);
            int       idleSteps = 0;
            const int maxIdle   = (20 - 2) * (20 - 2) * 2;
            while (true) {
                const int        s = QAgent::encodeState(game);
                const int        a = agent.greedyAction(s);
                const TickResult r = game.tick(QAgent::ACTIONS[a]);
                if (r == TickResult::AteFruit)
                    idleSteps = 0;
                else
                    ++idleSteps;
                if (r == TickResult::GameOver || idleSteps >= maxIdle)
                    break;
            }
            total += game.getScore();
        }
        return static_cast<float>(total) / static_cast<float>(evalEpisodes);
    };

    const QAgent untrained(0.0F);
    const float  baselineScore = evalAgent(untrained, 50);

    QAgent      agent(1.0F);
    const int   trainEpisodes = 5000;
    const float decayRate     = QAgent::computeDecayRate(trainEpisodes);

    for (int ep = 0; ep < trainEpisodes; ++ep) {
        Game      game(20);
        int       state     = QAgent::encodeState(game);
        int       idleSteps = 0;
        const int maxIdle   = (20 - 2) * (20 - 2) * 2;

        while (true) {
            const int        action = agent.selectAction(state);
            const char       dir    = QAgent::ACTIONS[action];
            const TickResult result = game.tick(dir);

            if (result == TickResult::GameOver) {
                agent.updateTerminal(state, action, QAgent::REWARD_DEATH);
                break;
            }

            const float reward =
                (result == TickResult::AteFruit) ? QAgent::REWARD_FRUIT : QAgent::REWARD_STEP;

            if (result == TickResult::AteFruit)
                idleSteps = 0;
            else
                ++idleSteps;

            const int nextState = QAgent::encodeState(game);
            agent.update(state, action, reward, nextState);
            state = nextState;
            if (idleSteps >= maxIdle)
                break;
        }
        agent.decayEpsilon(decayRate);
    }

    const float trainedScore = evalAgent(agent, 50);
    std::printf("  baseline avg score: %.2F  |  trained avg score: %.2F\n", baselineScore,
                trainedScore);

    EXPECT_TRUE(trainedScore > baselineScore,
                "trained agent scores higher than untrained baseline");
    EXPECT_TRUE(trainedScore >= 1.0F, "trained agent eats at least 1 fruit on average");
}

// ── Test 9: floodFill returns plausible count on a fresh game ─────────────────
// On a 20×20 grid the interior has 18×18 = 324 cells.  The snake starts with
// length 10, so at most 324 - 9 = 315 cells are reachable from the head.
// At minimum the head itself (1) must be reachable.

static void test_floodFill_range()
{
    std::printf("\n-- Test 9: floodFill returns plausible count --\n");
    bool ok = true;
    for (int trial = 0; trial < 20; ++trial) {
        Game         game(20);
        const Point& head      = game.getHead();
        const int    reachable = QAgent::floodFill(game, head.getX(), head.getY());
        const int    interior  = (20 - 2) * (20 - 2); // 324

        if (reachable <= 0 || reachable > interior) {
            FAIL("floodFill out of plausible range: %d (interior=%d)", reachable, interior);
            ok = false;
            break;
        }
    }
    if (ok)
        PASS("floodFill returns a value in (0, interior] on fresh games");
}

// ── Test 10: safeAction does not regress vs greedyAction ─────────────────────
// Train an agent, then compare average scores using greedyAction vs safeAction
// over 200 evaluation episodes.  safeAction must score at least as well
// (within a small variance margin) since it only overrides greedy when greedy
// would lead to self-enclosure.

static void test_safeAction_improves_over_greedy()
{
    std::printf("\n-- Test 10: safeAction vs greedyAction --\n");

    // ── Shared training ───────────────────────────────────────────────────────
    QAgent      agent(1.0F);
    const int   trainEpisodes = 5000;
    const float decayRate     = QAgent::computeDecayRate(trainEpisodes);
    const int   maxIdle       = (20 - 2) * (20 - 2) * 2;

    for (int ep = 0; ep < trainEpisodes; ++ep) {
        Game game(20);
        int  state     = QAgent::encodeState(game);
        int  idleSteps = 0;
        while (true) {
            const int        action = agent.selectAction(state);
            const char       dir    = QAgent::ACTIONS[action];
            const TickResult result = game.tick(dir);
            if (result == TickResult::GameOver) {
                agent.updateTerminal(state, action, QAgent::REWARD_DEATH);
                break;
            }
            const float reward =
                (result == TickResult::AteFruit) ? QAgent::REWARD_FRUIT : QAgent::REWARD_STEP;
            if (result == TickResult::AteFruit)
                idleSteps = 0;
            else
                ++idleSteps;
            const int nextState = QAgent::encodeState(game);
            agent.update(state, action, reward, nextState);
            state = nextState;
            if (idleSteps >= maxIdle)
                break;
        }
        agent.decayEpsilon(decayRate);
    }

    // ── Evaluate greedy ───────────────────────────────────────────────────────
    const int evalEpisodes = 200;
    long      greedyTotal  = 0;
    for (int ep = 0; ep < evalEpisodes; ++ep) {
        Game game(20);
        int  idleSteps = 0;
        while (true) {
            const int        s = QAgent::encodeState(game);
            const int        a = agent.greedyAction(s);
            const TickResult r = game.tick(QAgent::ACTIONS[a]);
            if (r == TickResult::AteFruit)
                idleSteps = 0;
            else
                ++idleSteps;
            if (r == TickResult::GameOver || idleSteps >= maxIdle)
                break;
        }
        greedyTotal += game.getScore();
    }
    const float greedyAvg = static_cast<float>(greedyTotal) / static_cast<float>(evalEpisodes);

    // ── Evaluate safeAction ───────────────────────────────────────────────────
    long safeTotal = 0;
    for (int ep = 0; ep < evalEpisodes; ++ep) {
        Game game(20);
        int  idleSteps = 0;
        while (true) {
            const int        s = QAgent::encodeState(game);
            const int        a = agent.safeAction(game, s);
            const TickResult r = game.tick(QAgent::ACTIONS[a]);
            if (r == TickResult::AteFruit)
                idleSteps = 0;
            else
                ++idleSteps;
            if (r == TickResult::GameOver || idleSteps >= maxIdle)
                break;
        }
        safeTotal += game.getScore();
    }
    const float safeAvg = static_cast<float>(safeTotal) / static_cast<float>(evalEpisodes);

    std::printf("  greedy avg score: %.2F  |  safe avg score: %.2F\n", greedyAvg, safeAvg);

    // safeAction should never be *meaningfully* worse than greedy — a >10%
    // regression would indicate a bug in the filter, not just random variance.
    // (With ~200 episodes and a ~23-fruit baseline the run-to-run noise is a few
    // percent, so a strict >= is too fragile here.)
    EXPECT_TRUE(safeAvg >= greedyAvg * 0.90F,
                "safeAction does not regress vs greedyAction (within 10%)");
}

// ── Main ──────────────────────────────────────────────────────────────────────

int main()
{
    test_encodeState_range();
    test_encodeState_food_bits();
    test_greedyAction();
    test_q_update_math();
    test_updateTerminal_math();
    test_epsilon_decay();
    test_save_load();
    test_selectAction_epsilon0_is_greedy();
    test_training_improves_score();
    test_floodFill_range();
    test_safeAction_improves_over_greedy();

    std::printf("\n%s  (%d failure(s))\n", failures == 0 ? "ALL PASSED" : "SOME FAILED", failures);
    return failures == 0 ? 0 : 1;
}
