#ifdef DQN_AVAILABLE

#include "DQNAgent.hpp"
#include "Game.hpp"
#include <cmath>
#include <cstdio>
#include <cstdlib>
#include <string>

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

// ── Test 1: encodeState produces [1, 20] tensor ────────────────────────────

static void test_encodeState_shape()
{
    std::printf("\n-- Test 1: encodeState output shape --\n");
    Game game(20);
    auto state = DQNAgent::encodeState(game);

    EXPECT_TRUE(state.dim() == 2, "encodeState tensor is 2-dimensional");
    EXPECT_TRUE(state.size(0) == 1, "encodeState batch dimension is 1");
    EXPECT_TRUE(state.size(1) == DQNNetImpl::INPUT_SIZE,
                "encodeState feature dimension matches INPUT_SIZE (20)");
}

// ── Test 2: encodeState features are within expected bounds ─────────────────
// All features should be in [-1, 1] by design:
//   [0-7]   danger flags: 0 or 1
//   [8-11]  direction one-hot: 0 or 1
//   [12-13] normalised food delta: [-1, 1]
//   [14]    normalised Manhattan distance: [0, 1]
//   [15]    normalised body length: [0, 1]
//   [16-19] flood-fill ratio: [0, 1]

static void test_encodeState_feature_ranges()
{
    std::printf("\n-- Test 2: encodeState feature ranges --\n");
    bool allInRange = true;
    for (int trial = 0; trial < 100; ++trial) {
        Game       game(20);
        const char dirs[] = {'w', 's', 'a', 'd'};
        for (int step = 0; step < 15; ++step) {
            if (game.tick(dirs[std::rand() % 4]) == TickResult::GameOver)
                break;
        }
        auto  state = DQNAgent::encodeState(game);
        auto  acc   = state.accessor<float, 2>();
        for (int i = 0; i < DQNNetImpl::INPUT_SIZE; ++i) {
            float v = acc[0][i];
            if (v < -1.01F || v > 1.01F) {
                FAIL("feature[%d] = %.4f out of [-1, 1] (trial %d)", i,
                     static_cast<double>(v), trial);
                allInRange = false;
                break;
            }
        }
        if (!allInRange)
            break;
    }
    if (allInRange)
        PASS("all 20 features in [-1, 1] across 100 random game states");
}

// ── Test 3: direction one-hot is valid ──────────────────────────────────────
// Features [8-11] should have exactly one 1.0 and three 0.0 values.

static void test_encodeState_direction_onehot()
{
    std::printf("\n-- Test 3: direction one-hot validity --\n");
    bool ok = true;
    const char dirs[] = {'w', 's', 'a', 'd'};
    for (int d = 0; d < 4; ++d) {
        Game game(20);
        game.tick(dirs[d]); // set a known direction
        auto state = DQNAgent::encodeState(game);
        auto acc   = state.accessor<float, 2>();

        int hotCount = 0;
        for (int i = 8; i < 12; ++i) {
            if (acc[0][i] == 1.0F)
                ++hotCount;
            else if (acc[0][i] != 0.0F) {
                FAIL("direction feature[%d] is %.2f, expected 0 or 1 (dir='%c')",
                     i, static_cast<double>(acc[0][i]), dirs[d]);
                ok = false;
                break;
            }
        }
        if (!ok)
            break;
        if (hotCount != 1) {
            FAIL("direction one-hot has %d hot bits, expected 1 (dir='%c')", hotCount, dirs[d]);
            ok = false;
            break;
        }
    }
    if (ok)
        PASS("direction one-hot has exactly one 1.0 for each direction");
}

// ── Test 4: action selection always in [0, 3] ──────────────────────────────

static void test_action_bounds()
{
    std::printf("\n-- Test 4: action selection bounds --\n");
    DQNAgent agent;
    bool     allValid = true;
    for (int trial = 0; trial < 200; ++trial) {
        Game game(20);
        auto state  = DQNAgent::encodeState(game);
        int  action = agent.selectAction(state, true);
        if (action < 0 || action >= DQNAgent::NUM_ACTIONS) {
            FAIL("selectAction returned %d, expected [0, %d)", action, DQNAgent::NUM_ACTIONS);
            allValid = false;
            break;
        }
        int greedy = agent.greedyAction(state);
        if (greedy < 0 || greedy >= DQNAgent::NUM_ACTIONS) {
            FAIL("greedyAction returned %d, expected [0, %d)", greedy, DQNAgent::NUM_ACTIONS);
            allValid = false;
            break;
        }
    }
    if (allValid)
        PASS("selectAction and greedyAction always return actions in [0, 4)");
}

// ── Test 5: training changes Q-values (network is not frozen) ───────────────
// Feed a batch of transitions with large positive reward and verify that the
// Q-value for the chosen action increases.

static void test_learning_signal()
{
    std::printf("\n-- Test 5: training updates Q-values --\n");
    DQNAgent agent;
    Game     game(20);
    auto     state = DQNAgent::encodeState(game);

    // Record greedy action before any training.
    int actionBefore = agent.greedyAction(state);

    // Feed transitions: action 0 gets +10 reward, all others get -10.
    // This creates a strong, unambiguous learning signal.
    const int totalSteps = DQNAgent::MIN_REPLAY + DQNAgent::TRAIN_EVERY * 100;
    for (int i = 0; i < totalSteps; ++i) {
        int   a = i % DQNAgent::NUM_ACTIONS;
        float r = (a == 0) ? 10.0F : -10.0F;
        agent.step(state, a, r, state, false);
    }

    // After 100 training batches with strong differential reward,
    // action 0 should dominate. Neural nets are stochastic so allow
    // some tolerance: check majority over multiple evaluations.
    int action0Count = 0;
    for (int trial = 0; trial < 20; ++trial) {
        if (agent.greedyAction(state) == 0)
            ++action0Count;
    }
    // greedyAction is deterministic (no dropout, no randomness), so
    // all 20 calls return the same value. We just need it to be 0.
    EXPECT_TRUE(action0Count == 20,
                "after differential reward, greedy consistently selects action 0");
}

// ── Test 6: save/load round-trip preserves greedy actions ───────────────────

static void test_save_load_roundtrip()
{
    std::printf("\n-- Test 6: save/load round-trip --\n");
    const std::string path = "/tmp/dqn_test_model.pt";

    DQNAgent agent;
    Game     game(20);
    auto     state = DQNAgent::encodeState(game);

    // Train a bit so weights are non-trivial.
    for (int i = 0; i < DQNAgent::MIN_REPLAY + DQNAgent::TRAIN_EVERY * 20; ++i)
        agent.step(state, i % 4, (i % 4 == 2) ? 10.0F : -1.0F, state, false);

    int actionBefore = agent.greedyAction(state);

    EXPECT_TRUE(agent.save(path), "save() returns true");

    // Load into a fresh agent.
    DQNAgent agent2;
    EXPECT_TRUE(agent2.load(path), "load() returns true");

    int actionAfter = agent2.greedyAction(state);
    EXPECT_TRUE(actionBefore == actionAfter,
                "greedy action matches after save/load round-trip");

    // Bad path returns false.
    DQNAgent agent3;
    EXPECT_TRUE(!agent3.load("/tmp/no_such_dqn_model_xyz.pt"),
                "load() returns false for missing file");

    std::remove(path.c_str());
}

// ── Test 7: epsilon decay reaches EPSILON_END ───────────────────────────────

static void test_epsilon_decay()
{
    std::printf("\n-- Test 7: epsilon decay --\n");
    DQNAgent    agent;
    const int   N    = 1000;
    const float rate = DQNAgent::computeDecayRate(N);
    for (int i = 0; i < N; ++i)
        agent.decayEpsilon(rate);

    float eps = agent.getEpsilon();
    EXPECT_TRUE(std::fabs(static_cast<double>(eps) - static_cast<double>(DQNAgent::EPSILON_END)) < 1e-3,
                "epsilon reaches EPSILON_END after N decays");

    // Further decay stays clamped.
    agent.decayEpsilon(0.5F);
    EXPECT_TRUE(agent.getEpsilon() >= DQNAgent::EPSILON_END,
                "epsilon stays clamped at EPSILON_END");
}

// ── Main ─────────────────────────────────────────────────────────────────────

int main()
{
    std::srand(42);

    test_encodeState_shape();
    test_encodeState_feature_ranges();
    test_encodeState_direction_onehot();
    test_action_bounds();
    test_learning_signal();
    test_save_load_roundtrip();
    test_epsilon_decay();

    std::printf("\n%s  (%d failure(s))\n", failures == 0 ? "ALL PASSED" : "SOME FAILED", failures);
    return failures == 0 ? 0 : 1;
}

#else // !DQN_AVAILABLE

#include <cstdio>

int main()
{
    std::printf("DQN not available (libtorch not found) — skipping tests.\n");
    return 0;
}

#endif // DQN_AVAILABLE
