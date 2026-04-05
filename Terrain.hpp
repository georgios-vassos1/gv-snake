#ifndef TERRAIN_H
#define TERRAIN_H

#include "Point.hpp"
#include "ShitList.hpp"
#include <pthread.h>

class Terrain
{
    char          **A;
    int             border;
    Point           fruit;
    ShitList        daCrap;
    pthread_mutex_t mutex;
    char            currentMove;

public:
    Terrain();
    ~Terrain();
    Terrain(const Terrain&)            = delete;
    Terrain& operator=(const Terrain&) = delete;

    void setDaBorder(int);
    int  getDaBorder() const;

    void createA();
    void deleteA();

    void fillA(bool type, bool frt);
    void draw() const;

    void         newFruit();
    const Point& getFruit() const;

    void makeDaShit();

    void*  moveDaShit(int tid);
    static void* moveDaShit_helper(void*);

    const ShitList& getDaCrap() const;
};

#endif
