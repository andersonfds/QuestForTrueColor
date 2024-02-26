
#define OLC_PGE_APPLICATION
#define OLC_IGNORE_VEC2D
#define TARGET_PHYSICS_PROCESS 60

#include <cassert>
#include <iostream>
#include <olcUTIL_Geometry2D.h>
#include <olcPixelGameEngine.h>
#include <LDtkLoader/Project.hpp>
#include "core/renderer.h"
#include "core/map.h"
#include "core/animation.h"
#include "core/bee.h"
#include "core/player.h"
#include "core/menu.h"
#include "core/coin.h"

Node *CreateEntity(const ldtk::Entity &entity)
{
    std::string name = entity.getName();

    if (name == "player")
        return new Player();

    if (name == "coin")
        return new Coin();

    if (name == "bee")
        return new Bee();

    return nullptr;
}

class QuestForTrueColor : public olc::PixelGameEngine
{
private:
    float fDeltaTime = 0.0f;
    Layer *gameLayer;

public:
    QuestForTrueColor()
    {
    }

    bool OnUserCreate() override
    {
        gameLayer = new Layer("Game", this);
        CreateMenu();
        return true;
    }

    void CreateMenu()
    {
        MainMenu *menu = new MainMenu(gameLayer);
        gameLayer->AddNode(menu);
        gameLayer->OnCreate();
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        fDeltaTime += fElapsedTime;

        if (GetKey(olc::Key::ESCAPE).bPressed)
        {
            OnUserCreate();
            return true;
        }

        if (GetKey(olc::Key::TAB).bPressed)
        {
            DEBUG = !DEBUG;
        }

        if (fDeltaTime >= 1.0f / TARGET_PHYSICS_PROCESS)
        {
            gameLayer->PhysicsProcess(fDeltaTime);
            fDeltaTime -= 1.0f / TARGET_PHYSICS_PROCESS;
        }

        gameLayer->Process(fElapsedTime);

        if (DEBUG)
        {
            std::string sFPS = "FPS: " + std::to_string(GetFPS());
            DrawStringDecal({10, 10}, sFPS, olc::WHITE);
        }

        return true;
    }
};
