#include "List.hpp"
#include <iostream>

ListNode::ListNode(int x, int y) : data(x, y), next(nullptr), prev(nullptr) {}

List::List() : first(nullptr), last(nullptr) {}

List::~List()
{
    deleteDaList();
}

ListNode* List::getFirst() const
{
    return first;
}
ListNode* List::getLast() const
{
    return last;
}

void List::insertFirst(const Point& p)
{
    ListNode* node = new ListNode(p.getX(), p.getY());
    if (isEmpty())
        last = node;
    else {
        node->next  = first;
        first->prev = node;
    }
    first = node;
}

void List::insertLast(const Point& p)
{
    ListNode* node = new ListNode(p.getX(), p.getY());
    if (isEmpty())
        first = node;
    else {
        last->next = node;
        node->prev = last;
    }
    last = node;
}

void List::deleteFirst()
{
    if (first == nullptr)
        return;
    ListNode* old = first;
    if (first->next == nullptr)
        last = nullptr;
    first = first->next;
    if (first != nullptr)
        first->prev = nullptr;
    delete old;
}

void List::deleteLast()
{
    if (first == nullptr)
        return;
    ListNode* old = last;
    if (first->next == nullptr)
        first = nullptr;
    else
        last->prev->next = nullptr;
    last = last->prev;
    delete old;
}

void List::deleteDaList()
{
    while (first != nullptr)
        deleteFirst();
}

bool List::isEmpty() const
{
    return first == nullptr;
}

void List::printDaList() const
{
    for (ListNode* n = first; n != nullptr; n = n->next)
        n->data.printDaPoint();
}
