#include "mygetch.hpp"
#include <stdio.h>
#include <sys/select.h>
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

bool kbhit()
{
    struct timeval tv = {0, 0};
    fd_set         fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, nullptr, nullptr, &tv) > 0;
}
