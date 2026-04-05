#include "HighScore.hpp"
#include <cstdio>
#include <cstdlib>
#include <cstring>

static const char* scorePath()
{
    static char path[512];
    const char* home = std::getenv("HOME");
    if (!home) home = ".";
    std::snprintf(path, sizeof(path), "%s/.snake_score", home);
    return path;
}

int loadHighScore()
{
    std::FILE* f = std::fopen(scorePath(), "r");
    if (!f) return 0;
    int val = 0;
    std::fscanf(f, "%d", &val);
    std::fclose(f);
    return val;
}

void saveHighScore(int score)
{
    std::FILE* f = std::fopen(scorePath(), "w");
    if (!f) return;
    std::fprintf(f, "%d\n", score);
    std::fclose(f);
}
