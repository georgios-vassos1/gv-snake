#include "Game.hpp"
#include "IRenderer.hpp"
#include "TerminalRenderer.hpp"
#include "mygetch.hpp"
#include <iostream>
#include <cstring>
#include <cstdio>
#include <cstdlib>

#ifdef GRAPHICS_AVAILABLE
#include "GraphicsRenderer.hpp"
#endif

static const int GRID_SIZE = 50;

static void cleanup()
{
    cleanupGetch();
    std::cout << "\033[?25h";  // restore terminal cursor
    std::cout.flush();
}

int main(int argc, char* argv[])
{
    bool useGraphics = false;
    for (int i = 1; i < argc; ++i) {
        if (std::strcmp(argv[i], "--graphics") == 0)
            useGraphics = true;
    }

    if (useGraphics) {
#ifdef GRAPHICS_AVAILABLE
        Game             game(GRID_SIZE);
        GraphicsRenderer renderer(GRID_SIZE);
        renderer.run(game);
#else
        std::fprintf(stderr,
            "Graphics mode not available (SDL2 not found at build time).\n"
            "Run without --graphics to use terminal mode.\n");
        return EXIT_FAILURE;
#endif
    } else {
        std::atexit(cleanup);
        initGetch();

        std::cout << "\033[2J\033[H\033[?25l";
        std::cout.flush();

        Game             game(GRID_SIZE);
        TerminalRenderer renderer;
        renderer.run(game);
    }

    return 0;
}
