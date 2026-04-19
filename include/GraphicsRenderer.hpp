#ifndef GRAPHICS_RENDERER_H
#define GRAPHICS_RENDERER_H

#include "IRenderer.hpp"

#ifdef GRAPHICS_AVAILABLE

#include <SDL2/SDL.h>

class QAgent;           // forward declaration — only needed when setAgent() is called
class HamiltonianAgent; // forward declaration — only needed when setHamiltonianAgent() is called
#ifdef DQN_AVAILABLE
class DQNAgent;
#endif

class GraphicsRenderer : public IRenderer {
    static const int CELL_SIZE = 12;
    static const int TICK_MS   = 70;

    SDL_Window*             window;
    SDL_Renderer*           sdlRenderer;
    int                     highScore;
    const QAgent*           agent_;       ///< non-null → Q-agent play
    const HamiltonianAgent* hamiltonian_; ///< non-null → Hamiltonian play
#ifdef DQN_AVAILABLE
    const DQNAgent*         dqnAgent_;    ///< non-null → DQN play
#endif

    void draw(const Game& game, int episode = 0) const;
    void runHuman(Game& game);
    void runAgent(int border);
    void runHamiltonian(int border);
#ifdef DQN_AVAILABLE
    void runDQNAgent(int border);
#endif

public:
    explicit GraphicsRenderer(int border);
    ~GraphicsRenderer() override;
    GraphicsRenderer(const GraphicsRenderer&)            = delete;
    GraphicsRenderer& operator=(const GraphicsRenderer&) = delete;

    /// Switch to Q-agent driven mode.
    void setAgent(const QAgent& agent) { agent_ = &agent; }

    /// Switch to Hamiltonian-agent driven mode.
    void setHamiltonianAgent(const HamiltonianAgent& h) { hamiltonian_ = &h; }

#ifdef DQN_AVAILABLE
    /// Switch to DQN-agent driven mode.
    void setDQNAgent(const DQNAgent& a) { dqnAgent_ = &a; }
#endif

    void run(Game& game) override;
};

#endif // GRAPHICS_AVAILABLE
#endif // GRAPHICS_RENDERER_H
