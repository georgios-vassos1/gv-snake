#include "AgentRenderer.hpp"
#include "HighScore.hpp"
#include <cstdio>
#include <iostream>
#include <unistd.h>

/// Pause between frames in microseconds (≈ 10 fps — comfortable to watch).
static constexpr int AGENT_TICK_US = 100000;

/// Maximum steps without eating fruit before the episode is abandoned.
/// Prevents the snake from looping forever when it finds a locally safe cycle.
static int maxIdleSteps(int border)
{
    const int interior = border - 2;
    return interior * interior * 2;
}

AgentRenderer::AgentRenderer(const QAgent& agent) : agent_(agent), highScore_(loadHighScore()) {}

void AgentRenderer::draw(const Game& game, int episode, int bestScore) const
{
    const int    b = game.getBorder();
    char* const* A = game.grid();

    std::cout << "\033[H";
    for (int i = 0; i < b; ++i) {
        for (int j = 0; j < b; ++j)
            std::cout << A[i][j];
        std::cout << '\n';
    }
    std::printf("Score: %d   Best: %d   Episode: %d\033[K\n", game.getScore(), bestScore, episode);
    std::cout.flush();
}

void AgentRenderer::run(Game& game)
{
    const int border = game.getBorder();

    int episode   = 0;
    int bestScore = highScore_;

    while (true) {
        ++episode;
        Game game(border);

        draw(game, episode, bestScore);

        int idleSteps = 0;

        while (true) {
            const int  state  = QAgent::encodeState(game);
            const int  action = agent_.safeAction(game, state);
            const char dir    = QAgent::ACTIONS[action];

            const TickResult result = game.tick(dir);
            draw(game, episode, bestScore);
            usleep(AGENT_TICK_US);

            if (result == TickResult::AteFruit) {
                idleSteps = 0;
            } else {
                ++idleSteps;
            }

            if (result == TickResult::GameOver) {
                const int score = game.getScore();
                if (score > bestScore) {
                    bestScore = score;
                    saveHighScore(score);
                }
                break;
            }

            if (idleSteps >= maxIdleSteps(border)) {
                // Agent is stuck in a loop — restart silently.
                break;
            }
        }
    }
}
