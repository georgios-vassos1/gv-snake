#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H

#include "IRenderer.hpp"

#ifdef GRAPHICS_AVAILABLE

#include <SDL2/SDL.h>

class GraphicsRenderer : public IRenderer {
    static const int CELL_SIZE = 12;
    static const int TICK_MS   = 70;

    SDL_Window*   window;
    SDL_Renderer* sdlRenderer;

    void draw(const Game& game) const;

public:
    explicit GraphicsRenderer(int border);
    ~GraphicsRenderer();
    GraphicsRenderer(const GraphicsRenderer&)            = delete;
    GraphicsRenderer& operator=(const GraphicsRenderer&) = delete;

    void run(Game& game) override;
};

#endif // GRAPHICS_AVAILABLE
#endif // GRAPHICS_RENDERER_H
