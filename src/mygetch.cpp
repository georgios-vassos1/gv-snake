#include "mygetch.hpp"
#include <stdio.h>
#include <termios.h>

static struct termios saved;

void initGetch()
{
    struct termios raw;
    tcgetattr(0, &saved);
    raw = saved;
    raw.c_lflag &= ~(ICANON | ECHO);
    tcsetattr(0, TCSANOW, &raw);
}

void cleanupGetch()
{
    tcsetattr(0, TCSANOW, &saved);
}

char getch()
{
    return static_cast<char>(getchar());
}
