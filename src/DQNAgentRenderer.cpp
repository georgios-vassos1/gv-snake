#ifdef DQN_AVAILABLE

#include "DQNAgentRenderer.hpp"
#include "HighScore.hpp"
#include <cstdio>
#include <iostream>
#include <unistd.h>

static constexpr int TICK_US = 100000; // ~10 fps

static int maxIdleSteps(int border)
{
    const int interior = border - 2;
    return interior * interior * 2;
}

DQNAgentRenderer::DQNAgentRenderer(const DQNAgent& agent)
    : agent_(agent), highScore_(loadHighScore())
{
}

void DQNAgentRenderer::draw(const Game& game, int episode, int bestScore) const
{
    const int    b = game.getBorder();
    char* const* A = game.grid();

    std::cout << "\033[H";
    for (int i = 0; i < b; ++i) {
        for (int j = 0; j < b; ++j)
            std::cout << A[i][j];
        std::cout << '\n';
    }
    std::printf("Score: %d   Best: %d   Episode: %d\033[K\n",
                game.getScore(), bestScore, episode);
    std::cout.flush();
}

void DQNAgentRenderer::run(Game& /*game*/)
{
    const int border  = 20;
    int       episode = 0;

    while (true) {
        ++episode;
        Game epGame(border);
        int  idleSteps = 0;

        draw(epGame, episode, highScore_);

        while (true) {
            auto       state  = DQNAgent::encodeState(epGame);
            const int  action = agent_.greedyAction(state);
            const char dir    = "wsad"[action];

            const TickResult result = epGame.tick(dir);
            draw(epGame, episode, highScore_);
            usleep(TICK_US);

            if (result == TickResult::AteFruit) {
                idleSteps = 0;
                if (epGame.getScore() > highScore_) {
                    highScore_ = epGame.getScore();
                    saveHighScore(highScore_);
                }
            } else {
                ++idleSteps;
            }

            if (result == TickResult::GameOver || idleSteps >= maxIdleSteps(border))
                break;
        }
    }
}

#endif // DQN_AVAILABLE
