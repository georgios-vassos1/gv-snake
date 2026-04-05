#include "Point.hpp"
#include <iostream>

Point::Point() : x(0), y(0) {}

Point::Point(int aX, int aY) : x(aX), y(aY) {}

int Point::getX() const { return x; }
int Point::getY() const { return y; }

void Point::setX(int aX) { x = aX; }
void Point::setY(int aY) { y = aY; }

void Point::moveUp()    { --x; }
void Point::moveDown()  { ++x; }
void Point::moveLeft()  { --y; }
void Point::moveRight() { ++y; }

void Point::printDaPoint() const
{
    std::cout << "|---> x = " << x << ", y = " << y << std::endl;
}
