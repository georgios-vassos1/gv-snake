#ifndef POINT_H
#define POINT_H

class Point {
    int x, y;

public:
    Point();
    Point(int, int);

    int  getX() const;
    int  getY() const;
    void setX(int);
    void setY(int);

    void moveUp();
    void moveDown();
    void moveLeft();
    void moveRight();

    void printDaPoint() const;
};

#endif
