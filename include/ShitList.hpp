#ifndef SHITLIST_H
#define SHITLIST_H

#include "List.hpp"

class ShitList
{
    List daShit;
    char lastMove;
    bool validness;

public:
    ShitList();
    ~ShitList();
    ShitList(const ShitList&)            = delete;
    ShitList& operator=(const ShitList&) = delete;

    void        deleteDaShit();
    const List& getDaShit() const;

    bool isFree(char, int);
    bool isFree(char, int, int);

    void setLastMove(char);
    char getLastMove()  const;
    void setValidness(bool);
    bool getValidness() const;

    void moveDaShitUp   (int);
    void moveDaShitDown (int);
    void moveDaShitLeft (int);
    void moveDaShitRight(int);

    void addPart(const Point&);
    void addPart(int, int);

private:
    Point nextHead(char, int) const;
    void  move(char, int);
};

#endif
