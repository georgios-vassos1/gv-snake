#include "Game.hpp"
#include <cstdlib>
#include <ctime>

const int  Game::INITIAL_LENGTH = 10;
const char Game::BORDER_CHAR    = '*';
const char Game::HEAD_CHAR      = 'A';
const char Game::BODY_CHAR      = 'O';
const char Game::FRUIT_CHAR     = 'X';
const char Game::EMPTY_CHAR     = ' ';

Game::Game(int b)
    : border(b), score(0), A(nullptr)
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    createA();
    for (int i = 0; i < INITIAL_LENGTH; i++)
        daCrap.addPart((border / 2) + i, border / 2);
    buildGrid(true);
}

Game::~Game()
{
    if (A != nullptr)
        deleteA();
}

void Game::createA()
{
    A = new char*[border];
    for (int i = 0; i < border; i++)
        A[i] = new char[border];
}

void Game::deleteA()
{
    for (int i = 0; i < border; i++)
        delete[] A[i];
    delete[] A;
    A = nullptr;
}

void Game::placeNewFruit()
{
    const int interior = border - 2;
    do {
        fruit.setX((std::rand() % interior) + 1);
        fruit.setY((std::rand() % interior) + 1);
    } while (A[fruit.getX()][fruit.getY()] != EMPTY_CHAR);
}

void Game::buildGrid(bool placeFruit)
{
    // Pass 1 — O(border²): borders and empty interior.
    for (int i = 0; i < border; i++)
        for (int j = 0; j < border; j++)
            A[i][j] = (i == 0 || i == border - 1 || j == 0 || j == border - 1)
                      ? BORDER_CHAR : EMPTY_CHAR;

    // Pass 2 — O(n): overlay the snake.
    for (ListNode* n = daCrap.getDaShit().getFirst(); n != nullptr; n = n->next)
        A[n->data.getX()][n->data.getY()] =
            (n == daCrap.getDaShit().getFirst()) ? HEAD_CHAR : BODY_CHAR;

    if (placeFruit)
        placeNewFruit();

    A[fruit.getX()][fruit.getY()] = FRUIT_CHAR;
}

TickResult Game::tick(char dir)
{
    if      (dir == 'w') daCrap.moveDaShitUp(border);
    else if (dir == 's') daCrap.moveDaShitDown(border);
    else if (dir == 'a') daCrap.moveDaShitLeft(border);
    else if (dir == 'd') daCrap.moveDaShitRight(border);

    if (!daCrap.getValidness())
        return TickResult::GameOver;

    ListNode* head = daCrap.getDaShit().getFirst();
    if (head->data.getX() == fruit.getX() && head->data.getY() == fruit.getY()) {
        // Grow: append a segment behind the head's previous position.
        Point growPt;
        if      (dir == 'w') growPt = Point(head->data.getX() + 1, head->data.getY());
        else if (dir == 's') growPt = Point(head->data.getX() - 1, head->data.getY());
        else if (dir == 'a') growPt = Point(head->data.getX(),     head->data.getY() + 1);
        else                 growPt = Point(head->data.getX(),     head->data.getY() - 1);
        daCrap.addPart(growPt);
        buildGrid(true);
        ++score;
        return TickResult::AteFruit;
    }

    buildGrid(false);
    return TickResult::Running;
}
