#ifndef MYGETCH_H
#define MYGETCH_H

// Call once at program start to put the terminal into raw/no-echo mode.
void initGetch();

// Call once at program exit to restore the original terminal settings.
void cleanupGetch();

// Read one character (terminal must already be initialised with initGetch).
char getch();

// Return true if a keypress is waiting in stdin (non-blocking).
bool kbhit();

#endif
