#ifndef TERMINAL_RENDERER_H
#define TERMINAL_RENDERER_H

#include "IRenderer.hpp"
#include <pthread.h>

class TerminalRenderer : public IRenderer {
    pthread_mutex_t mutex;
    pthread_t       inputThread;
    char            currentMove;
    int             highScore;

    static void* inputLoop(void* self);
    void         draw(const Game& game) const;

public:
    TerminalRenderer();
    ~TerminalRenderer() override;
    TerminalRenderer(const TerminalRenderer&)            = delete;
    TerminalRenderer& operator=(const TerminalRenderer&) = delete;

    void run(Game& game) override;
};

#endif
