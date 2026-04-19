#include "AgentRenderer.hpp"
#include "Game.hpp"
#include "QAgent.hpp"
#include "TerminalRenderer.hpp"
#include "mygetch.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <ctime>
#include <iostream>

#ifdef GRAPHICS_AVAILABLE
#include "GraphicsRenderer.hpp"
#endif

static const int GRID_SIZE = 20;

// ── Terminal setup/teardown ───────────────────────────────────────────────────

static void cleanup()
{
    cleanupGetch();
    std::cout << "\033[?25h"; // restore cursor
    std::cout.flush();
}

// ── Training loop ─────────────────────────────────────────────────────────────

/// Maximum steps without eating fruit before an episode is abandoned.
static int maxIdleSteps(int border)
{
    const int interior = border - 2;
    return interior * interior * 2;
}

static void runTraining(int episodes, const std::string& qtablePath)
{
    std::printf("Training: %d episodes on %dx%d grid\n", episodes, GRID_SIZE, GRID_SIZE);
    std::printf("  %-28s  %6s  %6s  %8s\n", "Progress", "AvgScr", "Best", "Epsilon");
    std::fflush(stdout);

    QAgent      agent(1.0F);
    const float decayRate = QAgent::computeDecayRate(episodes);

    int       bestScore   = 0;
    long      scoreAccum  = 0;
    const int reportEvery = std::max(1, episodes / 20); // 20 progress lines

    for (int ep = 1; ep <= episodes; ++ep) {
        Game game(GRID_SIZE);
        int  state     = QAgent::encodeState(game);
        int  idleSteps = 0;

        while (true) {
            const int  action = agent.selectAction(state);
            const char dir    = QAgent::ACTIONS[action];

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

            if (idleSteps >= maxIdleSteps(GRID_SIZE))
                break;
        }

        const int score = game.getScore();
        scoreAccum += score;
        if (score > bestScore)
            bestScore = score;

        agent.decayEpsilon(decayRate);

        if (ep % reportEvery == 0) {
            const float avg = static_cast<float>(scoreAccum) / static_cast<float>(reportEvery);
            std::printf("  Episode %*d/%d | avg: %6.2F | best: %4d | ε: %.4F\n",
                        static_cast<int>(std::to_string(episodes).size()), ep, episodes, avg,
                        bestScore, agent.getEpsilon());
            std::fflush(stdout);
            scoreAccum = 0;
        }
    }

    if (agent.save(qtablePath)) {
        std::printf("Q-table saved to %s\n", qtablePath.c_str());
    } else {
        std::fprintf(stderr, "ERROR: failed to save Q-table to %s\n", qtablePath.c_str());
    }
}

// ── Play loop (agent demo) ────────────────────────────────────────────────────

static void runPlay(const std::string& qtablePath, bool useGraphics)
{
    QAgent agent(0.0F); // pure greedy — no exploration
    if (!agent.load(qtablePath)) {
        std::fprintf(stderr,
                     "ERROR: could not load Q-table from '%s'.\n"
                     "Run with --train first to generate it.\n",
                     qtablePath.c_str());
        std::exit(EXIT_FAILURE);
    }

    Game game(GRID_SIZE);

    if (useGraphics) {
#ifdef GRAPHICS_AVAILABLE
        GraphicsRenderer renderer(GRID_SIZE);
        renderer.setAgent(agent);
        renderer.run(game);
#else
        std::fprintf(stderr, "Graphics mode not available (SDL2 not found at build time).\n"
                             "Run without --graphics to use terminal mode.\n");
        std::exit(EXIT_FAILURE);
#endif
    } else {
        std::atexit(cleanup);
        initGetch();
        std::cout << "\033[2J\033[H\033[?25l";
        std::cout.flush();

        AgentRenderer renderer(agent);
        renderer.run(game);
    }
}

// ── Usage ─────────────────────────────────────────────────────────────────────

static void printUsage(const char* argv0)
{
    std::fprintf(stderr,
                 "Usage:\n"
                 "  %s                                 human play (terminal)\n"
                 "  %s --graphics                      human play (SDL2, if available)\n"
                 "  %s --train [N]                     train for N episodes (default 10000)\n"
                 "  %s --train [N] --qtable <file>     save Q-table to <file>\n"
                 "  %s --play                          run trained agent (terminal)\n"
                 "  %s --play --graphics               run trained agent (SDL2)\n"
                 "  %s --play [--graphics] --qtable <file>  load Q-table from <file>\n",
                 argv0, argv0, argv0, argv0, argv0, argv0, argv0);
}

// ── Entry point ───────────────────────────────────────────────────────────────

int main(int argc, char* argv[])
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    bool        useGraphics = false;
    bool        doTrain     = false;
    bool        doPlay      = false;
    int         episodes    = 10000;
    std::string qtablePath  = "qtable.bin";

    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--graphics") == 0) {
            useGraphics = true;
        } else if (std::strcmp(argv[i], "--train") == 0) {
            doTrain = true;
            if (i + 1 < argc && argv[i + 1][0] != '-') {
                episodes = std::atoi(argv[++i]);
                if (episodes <= 0) {
                    std::fprintf(stderr, "ERROR: episode count must be positive\n");
                    return EXIT_FAILURE;
                }
            }
        } else if (std::strcmp(argv[i], "--play") == 0) {
            doPlay = true;
        } else if (std::strcmp(argv[i], "--qtable") == 0) {
            if (i + 1 >= argc) {
                std::fprintf(stderr, "ERROR: --qtable requires a path argument\n");
                return EXIT_FAILURE;
            }
            qtablePath = argv[++i];
        } else {
            std::fprintf(stderr, "Unknown option: %s\n", argv[i]);
            printUsage(argv[0]);
            return EXIT_FAILURE;
        }
    }

    if (doTrain && doPlay) {
        std::fprintf(stderr, "ERROR: --train and --play are mutually exclusive\n");
        return EXIT_FAILURE;
    }

    if (doTrain) {
        runTraining(episodes, qtablePath);
        return EXIT_SUCCESS;
    }

    if (doPlay) {
        runPlay(qtablePath, useGraphics);
        return EXIT_SUCCESS;
    }

    // ── Default: human play ───────────────────────────────────────────────────
    if (useGraphics) {
#ifdef GRAPHICS_AVAILABLE
        Game             game(GRID_SIZE);
        GraphicsRenderer renderer(GRID_SIZE);
        renderer.run(game);
#else
        std::fprintf(stderr, "Graphics mode not available (SDL2 not found at build time).\n"
                             "Run without --graphics to use terminal mode.\n");
        return EXIT_FAILURE;
#endif
    } else {
        std::atexit(cleanup);
        initGetch();
        std::cout << "\033[2J\033[H\033[?25l";
        std::cout.flush();

        Game             game(GRID_SIZE);
        TerminalRenderer renderer;
        renderer.run(game);
    }

    return EXIT_SUCCESS;
}
