#include "ShitList.hpp"

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

/// Compute the position the head will occupy after one step in direction dir,
/// wrapping at the playfield boundary.  Positions are 1-based within
/// [1, gridSize-2] on each axis.
Point ShitList::nextHead(char dir, int gridSize) const
{
    const int field = gridSize - 2;
    const int hx    = daShit.getFirst()->data.getX();
    const int hy    = daShit.getFirst()->data.getY();
    switch (dir) {
        case 'w': return Point((hx - 2 + field) % field + 1, hy);
        case 's': return Point( hx % field + 1,               hy);
        case 'a': return Point(hx, (hy - 2 + field) % field + 1);
        case 'd': return Point(hx,  hy % field + 1);
        default:  return daShit.getFirst()->data; // unreachable
    }
}

/// Returns true if the next head position (for dir) does not collide with any
/// body segment from the 4th node onward.  The tail node is excluded because
/// it will have moved away by the time the head arrives.
bool ShitList::isFree(char dir, int gridSize)
{
    if (!daShit.getFirst()) return false;

    const Point next = nextHead(dir, gridSize);
    ListNode *current = advanceN(daShit.getFirst(), 3);
    while (current) {
        if (current == daShit.getLast()) return true; // tail moves away; safe
        if (current->data.getX() == next.getX() &&
            current->data.getY() == next.getY())
            return false;
        current = current->next;
    }
    return true;
}

/// Returns true if the next head position (for dir) does not collide with the
/// neck (the 2nd segment).  Used to reject a 180-degree reversal.
bool ShitList::isFree(char dir, int gridSize, int /*type*/)
{
    if (!daShit.getFirst() || !daShit.getFirst()->next) return false;

    const Point next = nextHead(dir, gridSize);
    const Point& neck = daShit.getFirst()->next->data;
    return !(next.getX() == neck.getX() && next.getY() == neck.getY());
}

/// Core movement: validates the requested direction, falls back to the last
/// direction when the move would enter the neck, and executes the step.
void ShitList::move(char dir, int gridSize)
{
    if (!isFree(dir, gridSize)) {
        validness = false;
        return;
    }
    if (!isFree(dir, gridSize, 0)) {
        // Input would put head into its own neck; keep going in last direction.
        if (lastMove != dir && lastMove != 0)
            move(lastMove, gridSize);
        return;
    }
    daShit.deleteLast();
    daShit.insertFirst(nextHead(dir, gridSize));
    lastMove  = dir;
    validness = true;
}

void ShitList::moveDaShitUp   (int x) { move('w', x); }
void ShitList::moveDaShitDown (int x) { move('s', x); }
void ShitList::moveDaShitLeft (int y) { move('a', y); }
void ShitList::moveDaShitRight(int y) { move('d', y); }

void ShitList::addPart(const Point& location)
{
    daShit.insertLast(location);
}

void ShitList::addPart(int x, int y)
{
    Point location(x, y);
    daShit.insertLast(location);
}
