#define OLC_PGE_APPLICATION
#include <cassert>
#include <olcPixelGameEngine.h>
#include "player.cpp"

class QuestForTrueColor : public olc::PixelGameEngine
{
public:
    QuestForTrueColor()
    {
        sAppName = "Quest for True Color";
    }

public:
    player *m_Player;

    bool OnUserCreate() override
    {
        m_Player = new player();
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        Clear(olc::DARK_BLUE);

        m_Player->Render(this, fElapsedTime);

        // UI
        std::string fps = "FPS: " + std::to_string(GetFPS());
        DrawString(10, 10, fps, olc::WHITE);
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