#include "Terrain.hpp"
#include "Thread_Data.hpp"
#include "mygetch.hpp"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <pthread.h>

static const int NUM_THREADS = 2;
static const int GRID_SIZE   = 50;

static void cleanup()
{
    cleanupGetch();
    std::cout << "\033[?25h";  // restore terminal cursor
    std::cout.flush();
}

int main()
{
    std::atexit(cleanup);
    initGetch();

    // Clear screen, move cursor home, hide cursor.
    std::cout << "\033[2J\033[H\033[?25l";
    std::cout.flush();

    tdata args[NUM_THREADS];

    args[0].tid     = 0;
    args[0].carrier = new Terrain;
    args[0].carrier->setDaBorder(GRID_SIZE);
    args[0].carrier->fillA(true, true);
    args[0].carrier->draw();

    args[1].tid     = 1;
    args[1].carrier = args[0].carrier;

    pthread_t t[NUM_THREADS];
    for (int i = 0; i < NUM_THREADS; i++) {
        int rc = pthread_create(&t[i], nullptr, &Terrain::moveDaShit_helper, &args[i]);
        if (rc) {
            std::fprintf(stderr, "ERROR: pthread_create failed (code %d)\n", rc);
            return EXIT_FAILURE;
        }
    }

    pthread_exit(nullptr);
}
