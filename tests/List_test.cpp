#include "List.hpp"
#include "Point.hpp"
#include <iostream>

int main()
{
    using namespace std;

    List  L;
    Point k(1, 1), l(1, 2), m(2, 2), n(2, 1);

    L.insertLast(k);
    L.insertLast(l);
    L.insertLast(m);
    L.insertLast(n);

    L.printDaList();
    cout << "*************\n";
    L.deleteLast();
    L.printDaList();
    cout << "*************\n";

    return 0;
}
