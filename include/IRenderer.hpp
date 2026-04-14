#ifndef IRENDERER_H
#define IRENDERER_H

#include "Game.hpp"

class IRenderer {
public:
    virtual ~IRenderer()         = default;
    virtual void run(Game& game) = 0;
};

#endif
