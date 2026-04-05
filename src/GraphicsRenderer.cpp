#ifdef GRAPHICS_AVAILABLE

#include "GraphicsRenderer.hpp"
#include <cstdio>

// Colours (R, G, B, A)
static const SDL_Color COL_BORDER = {100, 100, 100, 255};
static const SDL_Color COL_EMPTY  = { 20,  20,  20, 255};
static const SDL_Color COL_HEAD   = { 50, 220,  50, 255};
static const SDL_Color COL_BODY   = { 30, 130,  30, 255};
static const SDL_Color COL_FRUIT  = {220,  50,  50, 255};

GraphicsRenderer::GraphicsRenderer(int border)
    : window(nullptr), sdlRenderer(nullptr)
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::fprintf(stderr, "SDL_Init error: %s\n", SDL_GetError());
        return;
    }
    const int winSize = border * CELL_SIZE;
    window = SDL_CreateWindow("Snake",
        SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
        winSize, winSize, 0);
    if (!window) {
        std::fprintf(stderr, "SDL_CreateWindow error: %s\n", SDL_GetError());
        SDL_Quit();
        return;
    }
    sdlRenderer = SDL_CreateRenderer(window, -1,
        SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (!sdlRenderer) {
        std::fprintf(stderr, "SDL_CreateRenderer error: %s\n", SDL_GetError());
        SDL_DestroyWindow(window);
        window = nullptr;
        SDL_Quit();
    }
}

GraphicsRenderer::~GraphicsRenderer()
{
    if (sdlRenderer) SDL_DestroyRenderer(sdlRenderer);
    if (window)      SDL_DestroyWindow(window);
    SDL_Quit();
}

void GraphicsRenderer::draw(const Game& game) const
{
    if (!sdlRenderer) return;

    const int     b = game.getBorder();
    char* const*  A = game.grid();

    SDL_SetRenderDrawColor(sdlRenderer,
        COL_EMPTY.r, COL_EMPTY.g, COL_EMPTY.b, COL_EMPTY.a);
    SDL_RenderClear(sdlRenderer);

    for (int row = 0; row < b; row++) {
        for (int col = 0; col < b; col++) {
            const SDL_Color* c = &COL_EMPTY;
            switch (A[row][col]) {
                case '*': c = &COL_BORDER; break;
                case 'A': c = &COL_HEAD;   break;
                case 'O': c = &COL_BODY;   break;
                case 'X': c = &COL_FRUIT;  break;
                default:  continue;  // empty — already cleared
            }
            SDL_SetRenderDrawColor(sdlRenderer, c->r, c->g, c->b, c->a);
            SDL_Rect rect = { col * CELL_SIZE, row * CELL_SIZE, CELL_SIZE, CELL_SIZE };
            SDL_RenderFillRect(sdlRenderer, &rect);
        }
    }
    SDL_RenderPresent(sdlRenderer);
}

void GraphicsRenderer::run(Game& game)
{
    if (!sdlRenderer) return;

    draw(game);

    char    currentDir = 0;
    Uint32  lastTick   = SDL_GetTicks();
    bool    running    = true;

    while (running) {
        SDL_Event e;
        while (SDL_PollEvent(&e)) {
            if (e.type == SDL_QUIT) {
                running = false;
                break;
            }
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_w: case SDLK_UP:    currentDir = 'w'; break;
                    case SDLK_s: case SDLK_DOWN:   currentDir = 's'; break;
                    case SDLK_a: case SDLK_LEFT:   currentDir = 'a'; break;
                    case SDLK_d: case SDLK_RIGHT:  currentDir = 'd'; break;
                    case SDLK_SPACE:               currentDir = ' '; break;
                    case SDLK_ESCAPE:              running = false;   break;
                    default: break;
                }
            }
        }
        if (!running) break;

        Uint32 now = SDL_GetTicks();
        if (now - lastTick >= static_cast<Uint32>(TICK_MS)) {
            lastTick = now;
            if (currentDir != 0 && currentDir != ' ') {
                TickResult result = game.tick(currentDir);
                draw(game);
                if (result == TickResult::GameOver) {
                    // Show game over — wait until window closed or Escape.
                    SDL_Event qe;
                    while (SDL_WaitEvent(&qe)) {
                        if (qe.type == SDL_QUIT) break;
                        if (qe.type == SDL_KEYDOWN &&
                            qe.key.keysym.sym == SDLK_ESCAPE) break;
                    }
                    running = false;
                }
            }
        }

        SDL_Delay(8);  // ~120 fps poll rate
    }
}

#endif // GRAPHICS_AVAILABLE
