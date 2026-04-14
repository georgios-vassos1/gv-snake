#include "List.hpp"
#include "Point.hpp"
#include <cstdio>

static int failures = 0;

#define EXPECT(cond, msg)                                                          \
    do {                                                                           \
        if (cond) {                                                                \
            std::printf("PASS  %s\n", msg);                                        \
        } else {                                                                   \
            std::printf("FAIL  %s\n", msg);                                        \
            ++failures;                                                            \
        }                                                                          \
    } while (0)

static int countForward(const List& L)
{
    int n = 0;
    for (const ListNode* p = L.getFirst(); p != nullptr; p = p->next)
        ++n;
    return n;
}

static int countBackward(const List& L)
{
    int n = 0;
    for (const ListNode* p = L.getLast(); p != nullptr; p = p->prev)
        ++n;
    return n;
}

int main()
{
    // ── Fresh list ────────────────────────────────────────────────────────────
    {
        List L;
        EXPECT(L.isEmpty(),             "fresh list: isEmpty() is true");
        EXPECT(L.getFirst() == nullptr, "fresh list: getFirst() is null");
        EXPECT(L.getLast()  == nullptr, "fresh list: getLast() is null");
    }

    // ── insertLast on empty list: first == last == the only node ──────────────
    {
        List  L;
        Point p(1, 2);
        L.insertLast(p);
        EXPECT(!L.isEmpty(),                           "insertLast: not empty");
        EXPECT(L.getFirst() == L.getLast(),            "insertLast on empty: first == last");
        EXPECT(L.getFirst()->data.getX() == 1
            && L.getFirst()->data.getY() == 2,         "insertLast on empty: node data correct");
    }

    // ── insertLast ordering: FIFO – head is first inserted, tail is last ──────
    {
        List  L;
        Point a(1, 0);
        Point b(2, 0);
        Point c(3, 0);
        L.insertLast(a);
        L.insertLast(b);
        L.insertLast(c);
        EXPECT(L.getFirst()->data.getX() == 1, "insertLast ordering: head is first inserted");
        EXPECT(L.getLast()->data.getX()  == 3, "insertLast ordering: tail is last inserted");
        EXPECT(countForward(L) == 3,           "insertLast ordering: forward count is 3");
    }

    // ── insertFirst ordering: each call prepends, so head is last inserted ────
    {
        List  L;
        Point a(1, 0);
        Point b(2, 0);
        Point c(3, 0);
        L.insertFirst(a);
        L.insertFirst(b);
        L.insertFirst(c);
        EXPECT(L.getFirst()->data.getX() == 3, "insertFirst ordering: head is last inserted");
        EXPECT(L.getLast()->data.getX()  == 1, "insertFirst ordering: tail is first inserted");
        EXPECT(countForward(L) == 3,           "insertFirst ordering: forward count is 3");
    }

    // ── Bidirectional link integrity ──────────────────────────────────────────
    // first->prev must always be null; last->next must always be null;
    // and the backward count must equal the forward count.
    {
        List  L;
        Point a(1, 0);
        Point b(2, 0);
        Point c(3, 0);
        L.insertLast(a);
        L.insertLast(b);
        L.insertLast(c);
        EXPECT(L.getFirst()->prev == nullptr, "link integrity: first->prev is null");
        EXPECT(L.getLast()->next  == nullptr, "link integrity: last->next is null");
        EXPECT(countForward(L) == countBackward(L),
               "link integrity: backward count equals forward count");
    }

    // ── deleteFirst: removes head, data shifts, new head's prev is null ───────
    // NOTE: the current deleteFirst() implementation does not reset
    // first->prev to nullptr after advancing the head pointer.  The
    // "new head's prev is null" assertion below will fail until that
    // one-liner is added to deleteFirst().
    {
        List  L;
        Point a(10, 0);
        Point b(20, 0);
        Point c(30, 0);
        L.insertLast(a);
        L.insertLast(b);
        L.insertLast(c);
        L.deleteFirst();
        EXPECT(L.getFirst()->data.getX() == 20, "deleteFirst: new head has correct data");
        EXPECT(countForward(L) == 2,             "deleteFirst: count decrements");
        EXPECT(L.getFirst()->prev == nullptr,    "deleteFirst: new head's prev is null");
    }

    // ── deleteLast: removes tail, data shifts, new tail's next is null ────────
    {
        List  L;
        Point a(10, 0);
        Point b(20, 0);
        Point c(30, 0);
        L.insertLast(a);
        L.insertLast(b);
        L.insertLast(c);
        L.deleteLast();
        EXPECT(L.getLast()->data.getX() == 20, "deleteLast: new tail has correct data");
        EXPECT(countForward(L) == 2,            "deleteLast: count decrements");
        EXPECT(L.getLast()->next == nullptr,    "deleteLast: new tail's next is null");
    }

    // ── Deleting the only element leaves an empty list ────────────────────────
    {
        List  L;
        L.insertLast(Point(5, 5));
        L.deleteFirst();
        EXPECT(L.isEmpty(),             "deleteFirst on single: isEmpty");
        EXPECT(L.getFirst() == nullptr, "deleteFirst on single: first is null");
        EXPECT(L.getLast()  == nullptr, "deleteFirst on single: last is null");
    }
    {
        List  L;
        L.insertLast(Point(5, 5));
        L.deleteLast();
        EXPECT(L.isEmpty(),             "deleteLast on single: isEmpty");
        EXPECT(L.getFirst() == nullptr, "deleteLast on single: first is null");
        EXPECT(L.getLast()  == nullptr, "deleteLast on single: last is null");
    }

    // ── deleteDaList empties a multi-element list ─────────────────────────────
    {
        List L;
        for (int i = 1; i <= 5; ++i)
            L.insertLast(Point(i, 0));
        L.deleteDaList();
        EXPECT(L.isEmpty(),             "deleteDaList: isEmpty afterward");
        EXPECT(L.getFirst() == nullptr, "deleteDaList: first is null");
        EXPECT(L.getLast()  == nullptr, "deleteDaList: last is null");
    }

    std::printf("\n%s  (%d failure(s))\n", failures == 0 ? "ALL PASSED" : "SOME FAILED", failures);
    return failures == 0 ? 0 : 1;
}
