#define OLC_PGE_APPLICATION
#include <cassert>
#include <olcPixelGameEngine.h>

class QuestForTrueColor : public olc::PixelGameEngine
{
public:
    QuestForTrueColor()
    {
        sAppName = "Quest for True Color";
    }

public:
    bool OnUserCreate() override
    {
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        Clear(olc::GREEN);
        return true;
    }
};

int main()
{
    QuestForTrueColor game;

    if (game.Construct(640, 360, 1, 1, true))
    {
        game.Start();
    }

    return 0;
}