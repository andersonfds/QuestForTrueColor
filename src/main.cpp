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
    std::vector<box *> m_Obstacles;

    bool OnUserCreate() override
    {
        m_Player = new player();

        m_Obstacles.push_back(new box(new olc::vf2d(100, 100), 32, 32));
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        Clear(olc::DARK_BLUE);

        m_Player->Render(this, fElapsedTime);

        for (auto &obstacle : m_Obstacles)
        {
            m_Player->OnHorizontalCollision(m_Player->getRaycastBox()->Intersects(obstacle), obstacle);
            m_Player->OnVerticalCollision(m_Player->getRaycastBoxVertical()->Intersects(obstacle), obstacle);
        }

        // UI
        std::string fps = "FPS: " + std::to_string(GetFPS());
        DrawString(10, 10, fps, olc::WHITE);

        // DEBUG DRAW OBSTACLES
        for (auto &obstacle : m_Obstacles)
        {
            DrawRect(obstacle->position->x, obstacle->position->y, obstacle->width, obstacle->height, olc::RED);
        }

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