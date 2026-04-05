#ifndef TERMINAL_RENDERER_H
#define TERMINAL_RENDERER_H

#include "IRenderer.hpp"
#include <pthread.h>

class TerminalRenderer : public IRenderer {
    pthread_mutex_t mutex;
    pthread_t       inputThread;
    char            currentMove;

    static void* inputLoop(void* self);
    void draw(const Game& game) const;

public:
    TerminalRenderer();
    ~TerminalRenderer();
    TerminalRenderer(const TerminalRenderer&)            = delete;
    TerminalRenderer& operator=(const TerminalRenderer&) = delete;

    void run(Game& game) override;
};

#endif
