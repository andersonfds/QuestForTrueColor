
#define OLC_SOUNDWAVE
#define OLC_PGE_APPLICATION
#define OLC_IGNORE_VEC2D
#define TARGET_PHYSICS_PROCESS 60

#include <cassert>
#include <iostream>
#include <SDL2/SDL.h>
#include <SDL2/SDL_mixer.h>
#include <audio.h>
#include <olcUTIL_Geometry2D.h>
#include <olcPixelGameEngine.h>
#include <LDtkLoader/Project.hpp>
#include "core/renderer.h"
#include "core/map.h"
#include "core/animation.h"
#include "core/player.h"
#include "core/items.h"
#include "core/bee.h"
#include "core/menu.h"
#include "core/coin.h"
#include "core/anderson.h"

Node *CreateEntity(const ldtk::Entity &entity)
{
    std::string name = entity.getName();

    if (name == "player")
        return new Player();

    if (name == "coin")
        return new Coin();

    if (name == "bee")
        return new Bee();

    if (name == "bug_spray")
        return new BugSpray();

    if (name == "purse")
        return new Purse();

    if (name == "checkpoint")
        return new Checkpoint();

    if (name == "anderson")
        return new Anderson();

    if (name == "flower")
        return new Flower();

    if (name == "portal")
        return new TeleportPoint();

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
