#include "Point.hpp"
#include <iostream>
using namespace std;

int main()
{
    Point a(1, 2);
    Point b(3, 4);

    cout << "|---> Point a\n";
    a.printDaPoint();
    cout << "|---> Point b\n";
    b.printDaPoint();
    cout << "|------------------------------|\n";

    a.moveDown();
    cout << "|---> Point a after moveDown\n";
    a.printDaPoint();

    a.moveRight();
    cout << "|---> Point a after moveRight\n";
    a.printDaPoint();

    a.moveUp();
    cout << "|---> Point a after moveUp\n";
    a.printDaPoint();

    a.moveLeft();
    cout << "|---> Point a after moveLeft (back to original)\n";
    a.printDaPoint();

    return 0;
}
