#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360

bool static DEBUG = true;

#include "src/game.cc"

int main()
{
    QuestForTrueColor game;

    if (game.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, 2, 2, false))
        game.Start();

    return 0;
}