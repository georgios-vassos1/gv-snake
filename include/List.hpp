#ifndef LIST_H
#define LIST_H

#include "Point.hpp"

struct ListNode {
    Point     data;
    ListNode* next;
    ListNode* prev;
    ListNode(int x, int y);
};

class List {
    ListNode* first;
    ListNode* last;

public:
    List();
    ~List();
    List(const List&)            = delete;
    List& operator=(const List&) = delete;

    ListNode* getFirst() const;
    ListNode* getLast() const;

    void insertFirst(const Point& p);
    void insertLast(const Point& p);
    void deleteFirst();
    void deleteLast();
    void deleteDaList();
    bool isEmpty() const;
    void printDaList() const;
};

#endif
