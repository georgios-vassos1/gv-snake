#include "ShitList.hpp"
#include <iostream>

/// Advance node pointer p forward n times, stopping early on nullptr.
static ListNode* advanceN(ListNode *p, int n)
{
    for (int i = 0; i < n && p != nullptr; i++)
        p = p->next;
    return p;
}

ShitList::ShitList() : lastMove(0), validness(true) {}

ShitList::~ShitList()
{
    daShit.deleteDaList();
}

void ShitList::deleteDaShit()
{
    daShit.deleteDaList();
}

const List& ShitList::getDaShit() const
{
    return daShit;
}

void ShitList::setValidness(bool v) { validness = v; }
bool ShitList::getValidness() const { return validness; }
void ShitList::setLastMove(char m)  { lastMove = m; }
char ShitList::getLastMove()  const { return lastMove; }

void ShitList::moveDaShitUp(int x)
{
    if (isFree('w', x)) {
        if (isFree('w', x, 0)) {
            Point newFirst(((x-4) + daShit.getFirst()->data.getX()) % (x-2) + 1,
                           daShit.getFirst()->data.getY());
            daShit.deleteLast();
            daShit.insertFirst(newFirst);
            setLastMove('w');
            validness = true;
        }
        else if (getLastMove() == 's') moveDaShitDown(x);
        else if (getLastMove() == 'a') moveDaShitLeft(x);
        else if (getLastMove() == 'd') moveDaShitRight(x);
    }
    else {
        validness = false;
        std::cout << "\nGame Over! Press any key to exit.\n";
    }
}

void ShitList::moveDaShitDown(int x)
{
    if (isFree('s', x)) {
        if (isFree('s', x, 0)) {
            Point newFirst((daShit.getFirst()->data.getX() % (x-2)) + 1,
                           daShit.getFirst()->data.getY());
            daShit.deleteLast();
            daShit.insertFirst(newFirst);
            setLastMove('s');
            validness = true;
        }
        else if (getLastMove() == 'w') moveDaShitUp(x);
        else if (getLastMove() == 'a') moveDaShitLeft(x);
        else if (getLastMove() == 'd') moveDaShitRight(x);
    }
    else {
        validness = false;
        std::cout << "\nGame Over! Press any key to exit.\n";
    }
}

void ShitList::moveDaShitLeft(int y)
{
    if (isFree('a', y)) {
        if (isFree('a', y, 0)) {
            Point newFirst(daShit.getFirst()->data.getX(),
                           ((y-4) + daShit.getFirst()->data.getY()) % (y-2) + 1);
            daShit.deleteLast();
            daShit.insertFirst(newFirst);
            setLastMove('a');
            validness = true;
        }
        else if (getLastMove() == 'w') moveDaShitUp(y);
        else if (getLastMove() == 's') moveDaShitDown(y);
        else if (getLastMove() == 'd') moveDaShitRight(y);
    }
    else {
        validness = false;
        std::cout << "\nGame Over! Press any key to exit.\n";
    }
}

void ShitList::moveDaShitRight(int y)
{
    if (isFree('d', y)) {
        if (isFree('d', y, 0)) {
            Point newFirst(daShit.getFirst()->data.getX(),
                           (daShit.getFirst()->data.getY() % (y-2)) + 1);
            daShit.deleteLast();
            daShit.insertFirst(newFirst);
            setLastMove('d');
            validness = true;
        }
        else if (getLastMove() == 'w') moveDaShitUp(y);
        else if (getLastMove() == 's') moveDaShitDown(y);
        else if (getLastMove() == 'a') moveDaShitLeft(y);
    }
    else {
        validness = false;
        std::cout << "\nGame Over! Press any key to exit.\n";
    }
}

bool ShitList::isFree(char m, int c)
{
    if (daShit.getFirst() == nullptr)
        return false;

    if (m == 'w') {
        if (daShit.getFirst()->data.getX() == 1) {
            ListNode *current = advanceN(daShit.getFirst(), 3);
            while (current != nullptr) {
                if (current == daShit.getLast())
                    return true;
                if (daShit.getFirst()->data.getX() == (c-2) - current->data.getX() + 1 &&
                    daShit.getFirst()->data.getY() == current->data.getY())
                    return false;
                current = current->next;
            }
            return true;
        }
        ListNode *current = advanceN(daShit.getFirst(), 3);
        while (current != nullptr) {
            if (current == daShit.getLast())
                return true;
            if (daShit.getFirst()->data.getX() == current->data.getX() + 1 &&
                daShit.getFirst()->data.getY() == current->data.getY())
                return false;
            current = current->next;
        }
        return true;
    }
    else if (m == 'a') {
        if (daShit.getFirst()->data.getY() == 1) {
            ListNode *current = advanceN(daShit.getFirst(), 3);
            while (current != nullptr) {
                if (daShit.getFirst()->data.getX() == current->data.getX() &&
                    daShit.getFirst()->data.getY() == (c-2) - current->data.getY() + 1 &&
                    current != daShit.getLast())
                    return false;
                current = current->next;
            }
            return true;
        }
        ListNode *current = advanceN(daShit.getFirst(), 3);
        while (current != nullptr) {
            if (daShit.getFirst()->data.getX() == current->data.getX() &&
                daShit.getFirst()->data.getY() == current->data.getY() + 1 &&
                current != daShit.getLast())
                return false;
            current = current->next;
        }
        return true;
    }
    else if (m == 'd') {
        if (daShit.getFirst()->data.getY() == c - 2) {
            ListNode *current = advanceN(daShit.getFirst(), 3);
            while (current != nullptr) {
                if (daShit.getFirst()->data.getX() == current->data.getX() &&
                    daShit.getFirst()->data.getY() == current->data.getY() + (c-2) - 1 &&
                    current != daShit.getLast())
                    return false;
                current = current->next;
            }
            return true;
        }
        ListNode *current = advanceN(daShit.getFirst(), 3);
        while (current != nullptr) {
            if (daShit.getFirst()->data.getX() == current->data.getX() &&
                daShit.getFirst()->data.getY() == current->data.getY() - 1 &&
                current != daShit.getLast())
                return false;
            current = current->next;
        }
        return true;
    }
    else if (m == 's') {
        if (daShit.getFirst()->data.getX() == c - 2) {
            ListNode *current = advanceN(daShit.getFirst(), 3);
            while (current != nullptr) {
                if (daShit.getFirst()->data.getX() == current->data.getX() + (c-2) - 1 &&
                    daShit.getFirst()->data.getY() == current->data.getY() &&
                    current != daShit.getLast())
                    return false;
                current = current->next;
            }
            return true;
        }
        ListNode *current = advanceN(daShit.getFirst(), 3);
        while (current != nullptr) {
            if (daShit.getFirst()->data.getX() == current->data.getX() - 1 &&
                daShit.getFirst()->data.getY() == current->data.getY() &&
                current != daShit.getLast())
                return false;
            current = current->next;
        }
        return true;
    }
    else {
        std::cout << "Move Not Valid!!!" << std::endl;
        return false;
    }
}

bool ShitList::isFree(char m, int c, int /*type*/)
{
    if (daShit.getFirst() == nullptr || daShit.getFirst()->next == nullptr)
        return false;

    if (m == 'w') {
        if (daShit.getFirst()->data.getX() == 1) {
            if (daShit.getFirst()->data.getX() == (c-2) - daShit.getFirst()->next->data.getX() + 1 &&
                daShit.getFirst()->data.getY() == daShit.getFirst()->next->data.getY())
                return false;
            return true;
        }
        if (daShit.getFirst()->data.getX() == daShit.getFirst()->next->data.getX() + 1 &&
            daShit.getFirst()->data.getY() == daShit.getFirst()->next->data.getY())
            return false;
        return true;
    }
    else if (m == 'a') {
        if (daShit.getFirst()->data.getY() == 1) {
            if (daShit.getFirst()->data.getX() == daShit.getFirst()->next->data.getX() &&
                daShit.getFirst()->data.getY() == (c-2) - daShit.getFirst()->next->data.getY() + 1)
                return false;
            return true;
        }
        if (daShit.getFirst()->data.getX() == daShit.getFirst()->next->data.getX() &&
            daShit.getFirst()->data.getY() == daShit.getFirst()->next->data.getY() + 1)
            return false;
        return true;
    }
    else if (m == 'd') {
        if (daShit.getFirst()->data.getY() == c - 2) {
            if (daShit.getFirst()->data.getX() == daShit.getFirst()->next->data.getX() &&
                daShit.getFirst()->data.getY() == daShit.getFirst()->next->data.getY() + (c-2) - 1)
                return false;
            return true;
        }
        if (daShit.getFirst()->data.getX() == daShit.getFirst()->next->data.getX() &&
            daShit.getFirst()->data.getY() == daShit.getFirst()->next->data.getY() - 1)
            return false;
        return true;
    }
    else if (m == 's') {
        if (daShit.getFirst()->data.getX() == c - 2) {
            if (daShit.getFirst()->data.getX() == daShit.getFirst()->next->data.getX() + (c-2) - 1 &&
                daShit.getFirst()->data.getY() == daShit.getFirst()->next->data.getY())
                return false;
            return true;
        }
        if (daShit.getFirst()->data.getX() == daShit.getFirst()->next->data.getX() - 1 &&
            daShit.getFirst()->data.getY() == daShit.getFirst()->next->data.getY())
            return false;
        return true;
    }
    else {
        std::cout << "Move Not Valid!!!" << std::endl;
        return false;
    }
}

void ShitList::addPart(const Point& location)
{
    daShit.insertLast(location);
}

void ShitList::addPart(int x, int y)
{
    Point location(x, y);
    daShit.insertLast(location);
}
