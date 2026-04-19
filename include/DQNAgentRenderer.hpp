#ifndef DQN_AGENT_RENDERER_H
#define DQN_AGENT_RENDERER_H

#ifdef DQN_AVAILABLE

#include "DQNAgent.hpp"
#include "IRenderer.hpp"

/// Terminal renderer driven by the DQN agent (mirrors AgentRenderer).
class DQNAgentRenderer : public IRenderer {
    const DQNAgent& agent_;
    int             highScore_;

    void draw(const Game& game, int episode, int bestScore) const;

public:
    explicit DQNAgentRenderer(const DQNAgent& agent);
    void run(Game& game) override;
};

#endif // DQN_AVAILABLE
#endif // DQN_AGENT_RENDERER_H
