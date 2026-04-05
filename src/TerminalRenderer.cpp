#include "TerminalRenderer.hpp"
#include "HighScore.hpp"
#include "mygetch.hpp"
#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <pthread.h>

static const int TICK_US = 70000;

TerminalRenderer::TerminalRenderer()
    : inputThread(0), currentMove(0), highScore(loadHighScore())
{
    pthread_mutex_init(&mutex, nullptr);
}

TerminalRenderer::~TerminalRenderer()
{
    pthread_mutex_destroy(&mutex);
}

void* TerminalRenderer::inputLoop(void* self)
{
    TerminalRenderer* tr = static_cast<TerminalRenderer*>(self);
    while (true) {
        char key = getch();
        if (key == 'w' || key == 'a' || key == 's' || key == 'd' || key == ' ') {
            pthread_mutex_lock(&tr->mutex);
            tr->currentMove = key;
            pthread_mutex_unlock(&tr->mutex);
        }
    }
    return nullptr;
}

void TerminalRenderer::draw(const Game& game) const
{
    const int b      = game.getBorder();
    char* const* A   = game.grid();
    std::cout << "\033[H";
    for (int i = 0; i < b; i++) {
        for (int j = 0; j < b; j++)
            std::cout << A[i][j];
        std::cout << '\n';
    }
    std::printf("Score: %d   Best: %d\033[K\n", game.getScore(), highScore);
    std::cout.flush();
}

void TerminalRenderer::run(Game& game)
{
    draw(game);

    int rc = pthread_create(&inputThread, nullptr, inputLoop, this);
    if (rc) {
        std::fprintf(stderr, "ERROR: pthread_create failed (code %d)\n", rc);
        return;
    }

    while (true) {
        pthread_mutex_lock(&mutex);
        char dir = currentMove;
        pthread_mutex_unlock(&mutex);

        if (dir == 0 || dir == ' ') {
            usleep(TICK_US);
            continue;
        }

        TickResult result = game.tick(dir);
        draw(game);

        if (result == TickResult::GameOver)
            break;

        usleep(TICK_US);
    }

    pthread_cancel(inputThread);
    pthread_join(inputThread, nullptr);

    const int finalScore = game.getScore();
    if (finalScore > highScore) {
        saveHighScore(finalScore);
        std::printf("\nNew high score: %d!\n", finalScore);
    } else {
        std::printf("\nHigh score: %d\n", highScore);
    }
    std::cout << "Press any key to exit.\n";
    std::cout.flush();
    getch();
}
