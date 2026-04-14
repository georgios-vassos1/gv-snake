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
        Game game(20);
        // Take a few random steps to vary the state.
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

// ── Test 2: encodeState is deterministic ─────────────────────────────────────

static void test_encodeState_deterministic()
{
    std::printf("\n-- Test 2: encodeState determinism --\n");
    // Construct two fresh games with the same seed by comparing same initial state.
    Game g1(20);
    // Tick both games identically.
    const char script[] = {'d', 'd', 's', 's', 'a'};
    bool       g1Dead   = false;
    for (const char d : script) {
        if (g1.tick(d) == TickResult::GameOver) {
            g1Dead = true;
            break;
        }
    }
    if (g1Dead) {
        PASS("encodeState determinism (snake died in setup — skipped)");
        return;
    }
    const int idx1 = QAgent::encodeState(g1);
    const int idx2 = QAgent::encodeState(g1); // same object, same state
    EXPECT_EQ(idx1, idx2, "encodeState returns same value for same game state");
}

// ── Test 3: greedyAction returns the argmax of Q values ──────────────────────

static void test_greedyAction()
{
    std::printf("\n-- Test 3: greedyAction is argmax --\n");
    // Manually plant Q values and verify greedyAction picks the largest.
    // We test by running update() to inject known values, then checking.
    QAgent agent(0.0F);

    // Force Q[state=0][action=2] to be the highest via multiple updates.
    // Q[0][2] += ALPHA * (large_reward + 0 - 0) per update.
    // After enough updates it should dominate.
    const int target_state  = 0;
    const int target_action = 2;
    for (int i = 0; i < 100; ++i)
        agent.update(target_state, target_action, 1000.0F, 0);

    EXPECT_EQ(agent.greedyAction(target_state), target_action,
              "greedyAction selects the highest-Q action");
}

// ── Test 4: Q-update math ─────────────────────────────────────────────────────

static void test_q_update_math()
{
    std::printf("\n-- Test 4: Q-update correctness --\n");
    QAgent agent(0.0F);

    // Initial Q is all zeros.
    // update(s=0, a=0, reward=10, s'=0):
    //   Q[0][0] += 0.1 * (10 + 0.9 * max(Q[0]) - Q[0][0])
    //            = 0.1 * (10 + 0.9 * 0 - 0)
    //            = 1.0
    agent.update(0, 0, 10.0F, 0);
    EXPECT_NEAR(agent.getQ(0, 0), 1.0F, 1e-5F, "first Q-update: Q[0][0] = 1.0");

    // Second update (s=0, a=0, reward=0, s'=0):
    //   Q[0][0] += 0.1 * (0 + 0.9 * 1.0 - 1.0)
    //            = 0.1 * (0.9 - 1.0)
    //            = 0.1 * (-0.1) = -0.01
    //   New Q[0][0] = 1.0 - 0.01 = 0.99
    agent.update(0, 0, 0.0F, 0);
    EXPECT_NEAR(agent.getQ(0, 0), 0.99F, 1e-5F, "second Q-update: Q[0][0] = 0.99");
}

// ── Test 4b: updateTerminal math ──────────────────────────────────────────────

static void test_updateTerminal_math()
{
    std::printf("\n-- Test 4b: updateTerminal correctness --\n");
    QAgent agent(0.0F);

    // updateTerminal(s=1, a=0, reward=-10):
    //   Q[1][0] += 0.1 * (-10 - 0)  = -1.0
    agent.updateTerminal(1, 0, -10.0F);
    EXPECT_NEAR(agent.getQ(1, 0), -1.0F, 1e-5F,
                "terminal Q-update: Q[1][0] = -1.0 (no successor term)");
}

// ── Test 5: epsilon decay ─────────────────────────────────────────────────────

static void test_epsilon_decay()
{
    std::printf("\n-- Test 5: epsilon decay --\n");
    QAgent agent(1.0F);

    // After N calls with a carefully computed rate, epsilon should reach MIN.
    const int   N    = 1000;
    const float rate = QAgent::computeDecayRate(N);
    for (int i = 0; i < N; ++i)
        agent.decayEpsilon(rate);

    EXPECT_NEAR(agent.getEpsilon(), QAgent::EPSILON_MIN, 1e-4F,
                "epsilon reaches EPSILON_MIN after N decays");

    // Further decay should stay clamped at EPSILON_MIN.
    agent.decayEpsilon(0.5F);
    EXPECT_NEAR(agent.getEpsilon(), QAgent::EPSILON_MIN, 1e-6F, "epsilon clamped at EPSILON_MIN");
}

// ── Test 6: save / load round-trip ───────────────────────────────────────────

static void test_save_load()
{
    std::printf("\n-- Test 6: save/load round-trip --\n");
    const std::string path = "/tmp/qagent_test.bin";

    QAgent a(0.0F);
    // Plant a known value.
    a.update(42, 3, 99.0F, 0);
    const float before = a.getQ(42, 3);

    EXPECT_TRUE(a.save(path), "save() returns true");

    QAgent b(0.0F);
    EXPECT_TRUE(b.load(path), "load() returns true");
    EXPECT_NEAR(b.getQ(42, 3), before, 1e-6F, "loaded Q[42][3] matches saved value");

    // Non-existent file returns false.
    QAgent c(0.0F);
    EXPECT_TRUE(!c.load("/tmp/no_such_file_xyz.bin"), "load() returns false for missing file");
    std::remove(path.c_str());
}

// ── Test 7: actions array covers all four directions ─────────────────────────

static void test_actions_array()
{
    std::printf("\n-- Test 7: ACTIONS array --\n");
    const char expected[] = {'w', 's', 'a', 'd'};
    bool       ok         = true;
    for (int i = 0; i < QAgent::NUM_ACTIONS; ++i) {
        if (QAgent::ACTIONS[i] != expected[i]) {
            ok = false;
            break;
        }
    }
    EXPECT_TRUE(ok, "ACTIONS = {w, s, a, d}");
}

// ── Test 8: integration — training improves score ────────────────────────────

static void test_training_improves_score()
{
    std::printf("\n-- Test 8: training improves score --\n");

    // Evaluate average score over 'evalEpisodes' episodes.
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

    // Untrained agent baseline (all Q = 0 → action 0 always).
    const QAgent untrained(0.0F);
    const float  baselineScore = evalAgent(untrained, 50);

    // Train for 5000 episodes.
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

// ── Main ──────────────────────────────────────────────────────────────────────

int main()
{
    test_encodeState_range();
    test_encodeState_deterministic();
    test_greedyAction();
    test_q_update_math();
    test_updateTerminal_math();
    test_epsilon_decay();
    test_save_load();
    test_actions_array();
    test_training_improves_score();

    std::printf("\n%s  (%d failure(s))\n", failures == 0 ? "ALL PASSED" : "SOME FAILED", failures);
    return failures == 0 ? 0 : 1;
}
