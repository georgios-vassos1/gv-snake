#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H

#include "IRenderer.hpp"

#ifdef GRAPHICS_AVAILABLE

#include <SDL2/SDL.h>

class QAgent; // forward declaration — only needed when setAgent() is called

class GraphicsRenderer : public IRenderer {
    static const int CELL_SIZE = 12;
    static const int TICK_MS   = 70;

    SDL_Window*   window;
    SDL_Renderer* sdlRenderer;
    int           highScore;
    const QAgent* agent_; ///< nullptr → human play; non-null → agent play

    void draw(const Game& game, int episode = 0) const;
    void runHuman(Game& game);
    void runAgent(int border);

public:
    explicit GraphicsRenderer(int border);
    ~GraphicsRenderer() override;
    GraphicsRenderer(const GraphicsRenderer&)            = delete;
    GraphicsRenderer& operator=(const GraphicsRenderer&) = delete;

    /// Call before run() to switch to agent-driven mode.
    void setAgent(const QAgent& agent) { agent_ = &agent; }

    void run(Game& game) override;
};

#endif // GRAPHICS_AVAILABLE
#endif // GRAPHICS_RENDERER_H
