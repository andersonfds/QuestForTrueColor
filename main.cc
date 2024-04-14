#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 360

bool static DEBUG = true;

#include "src/game.cc"

int main()
{
    QuestForTrueColor game;
    srand(time(NULL));

    if (SDL_Init(SDL_INIT_AUDIO) < 0)
    {
        std::cout << "SDL could not initialize! SDL_Error: " << SDL_GetError() << std::endl;
        return 1;
    }

    if (game.Construct(SCREEN_WIDTH, SCREEN_HEIGHT, 2, 2, false))
        game.Start();

    return 0;
}