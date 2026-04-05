#include "Terrain.hpp"
#include "Thread_Data.hpp"
#include "mygetch.hpp"
#include <iostream>
#include <cstdlib>
#include <ctime>
#include <unistd.h>
#include <pthread.h>

static const int  INITIAL_LENGTH = 10;
static const int  TICK_US        = 70000;
static const char BORDER_CHAR    = '*';
static const char HEAD_CHAR      = 'A';
static const char BODY_CHAR      = 'O';
static const char FRUIT_CHAR     = 'X';
static const char EMPTY_CHAR     = ' ';

Terrain::Terrain()
    : A(nullptr), border(0), currentMove(0)
{
    pthread_mutex_init(&mutex, nullptr);
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
}

Terrain::~Terrain()
{
    if (A != nullptr)
        deleteA();
    pthread_mutex_destroy(&mutex);
}

void Terrain::setDaBorder(int b) { border = b; }
int  Terrain::getDaBorder() const { return border; }

void Terrain::createA()
{
    A = new char*[border];
    for (int i = 0; i < border; i++)
        A[i] = new char[border];
}

void Terrain::deleteA()
{
    for (int i = 0; i < border; i++)
        delete[] A[i];
    delete[] A;
    A = nullptr;
}

void Terrain::draw() const
{
    // Cursor home: overwrite the grid in-place instead of appending new lines.
    std::cout << "\033[H";
    for (int i = 0; i < border; i++) {
        for (int j = 0; j < border; j++)
            std::cout << A[i][j];
        std::cout << '\n';
    }
    std::cout.flush();
}

void Terrain::makeDaShit()
{
    for (int i = 0; i < INITIAL_LENGTH; i++)
        daCrap.addPart((border / 2) + i, border / 2);
}

void Terrain::newFruit()
{
    // Interior cells are rows/cols 1 .. border-2.
    const int interior = border - 2;
    do {
        fruit.setX((std::rand() % interior) + 1);
        fruit.setY((std::rand() % interior) + 1);
    } while (A[fruit.getX()][fruit.getY()] != EMPTY_CHAR);
}

const Point& Terrain::getFruit() const { return fruit; }

void Terrain::fillA(bool type, bool frt)
{
    if (type) {
        makeDaShit();
        createA();
    }

    // Pass 1 — O(border²): borders and empty interior.
    for (int i = 0; i < border; i++) {
        for (int j = 0; j < border; j++) {
            A[i][j] = (i == 0 || i == border - 1 || j == 0 || j == border - 1)
                      ? BORDER_CHAR : EMPTY_CHAR;
        }
    }

    // Pass 2 — O(n): overlay the snake.
    for (ListNode *n = daCrap.getDaShit().getFirst(); n != nullptr; n = n->next)
        A[n->data.getX()][n->data.getY()] =
            (n == daCrap.getDaShit().getFirst()) ? HEAD_CHAR : BODY_CHAR;

    // Fruit: place a new one when needed, otherwise redraw the existing one.
    if ((type && frt) || (!type && !frt))
        newFruit();

    A[fruit.getX()][fruit.getY()] = FRUIT_CHAR;
}

void* Terrain::moveDaShit(int tid)
{
    if (tid == 1) {
        // Input thread: blocking read, update direction under the mutex.
        while (true) {
            char key = getch();
            if (key == 'w' || key == 'a' || key == 's' || key == 'd' || key == ' ') {
                pthread_mutex_lock(&mutex);
                currentMove = key;
                pthread_mutex_unlock(&mutex);
            }
        }
        pthread_exit(nullptr);
    }

    // Game-loop thread (tid == 0): advance snake every TICK_US microseconds.
    while (true) {
        pthread_mutex_lock(&mutex);
        char dir = currentMove;
        pthread_mutex_unlock(&mutex);

        // Wait for first keypress or honour pause.
        if (dir == 0 || dir == ' ') {
            usleep(TICK_US);
            continue;
        }

        if      (dir == 'w') daCrap.moveDaShitUp(border);
        else if (dir == 's') daCrap.moveDaShitDown(border);
        else if (dir == 'a') daCrap.moveDaShitLeft(border);
        else if (dir == 'd') daCrap.moveDaShitRight(border);

        if (!daCrap.getValidness()) {
            draw();
            std::cout << "\nGame Over!  Press any key to exit.\n";
            std::cout.flush();
            getch();
            std::exit(0);
        }

        // Grow the snake when the head lands on the fruit.
        ListNode *head = daCrap.getDaShit().getFirst();
        if (head->data.getX() == fruit.getX() && head->data.getY() == fruit.getY()) {
            // Add a new tail segment at the position the head vacated.
            Point growPt;
            if      (dir == 'w') growPt = Point(head->data.getX() + 1, head->data.getY());
            else if (dir == 's') growPt = Point(head->data.getX() - 1, head->data.getY());
            else if (dir == 'a') growPt = Point(head->data.getX(),     head->data.getY() + 1);
            else                 growPt = Point(head->data.getX(),     head->data.getY() - 1);
            daCrap.addPart(growPt);
            fillA(false, false);   // rebuild grid and place a new fruit
        }
        else {
            fillA(false, true);    // rebuild grid, keep the existing fruit
        }

        draw();
        usleep(TICK_US);
    }
    pthread_exit(nullptr);
}

void* Terrain::moveDaShit_helper(void *args)
{
    tdata *my_data = static_cast<tdata*>(args);
    my_data->carrier->moveDaShit(my_data->tid);
    pthread_exit(nullptr);
}

const ShitList& Terrain::getDaCrap() const { return daCrap; }
