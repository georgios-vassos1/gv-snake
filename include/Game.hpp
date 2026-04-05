#ifndef GAME_H
#define GAME_H

#include "ShitList.hpp"

enum class TickResult { Running, GameOver, AteFruit };

class Game {
    int      border;
    char**   A;
    ShitList daCrap;
    Point    fruit;

    static const int  INITIAL_LENGTH;
    static const char BORDER_CHAR;
    static const char HEAD_CHAR;
    static const char BODY_CHAR;
    static const char FRUIT_CHAR;
    static const char EMPTY_CHAR;

    void createA();
    void deleteA();
    void buildGrid(bool placeFruit);
    void placeNewFruit();

public:
    explicit Game(int border);
    ~Game();
    Game(const Game&)            = delete;
    Game& operator=(const Game&) = delete;

    int          getBorder() const { return border; }
    const Point& getFruit()  const { return fruit;  }
    char* const* grid()      const { return A;       }

    TickResult tick(char dir);
};

#endif
