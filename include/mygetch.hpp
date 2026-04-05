#ifndef MYGETCH_H
#define MYGETCH_H

// Call once at program start to put the terminal into raw/no-echo mode.
void initGetch();

// Call once at program exit to restore the original terminal settings.
void cleanupGetch();

// Read one character (terminal must already be initialised with initGetch).
char getch();

#endif
