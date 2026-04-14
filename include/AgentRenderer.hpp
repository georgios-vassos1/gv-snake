#ifndef AGENT_RENDERER_H
#define AGENT_RENDERER_H

#include "IRenderer.hpp"
#include "QAgent.hpp"

/// Drives the game using a trained QAgent, rendering each frame to the
/// terminal.  Drop-in replacement for TerminalRenderer in --play mode.
class AgentRenderer : public IRenderer {
    const QAgent& agent_;
    int           highScore_;

    void draw(const Game& game, int episode, int bestScore) const;

public:
    /// agent must outlive this object.
    explicit AgentRenderer(const QAgent& agent);
    ~AgentRenderer() override                      = default;
    AgentRenderer(const AgentRenderer&)            = delete;
    AgentRenderer& operator=(const AgentRenderer&) = delete;

    /// Run indefinitely, restarting after each game-over.
    /// Each new game is constructed internally using the same border size as
    /// the game passed in; the caller's game object is used only to read the
    /// border size.  Press Ctrl-C to exit (atexit handler restores terminal).
    void run(Game& game) override;
};

#endif // AGENT_RENDERER_H
