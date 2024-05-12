#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360
#define OLC_SOUNDWAVE
#include <olcSoundWaveEngine.h>

bool static DEBUG = false;

#include "src/game.cc"

int main()
{
    QuestForTrueColor game;
    srand(time(NULL));

    if (game.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, 2, 2, false))
        game.Start();

    return 0;
}