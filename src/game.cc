
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
#include "core/player.h"
#include "core/scene.h"

class QuestForTrueColor : public olc::PixelGameEngine
{
private:
    float fDeltaTime = 0.0f;
    Layer *mapLayer;
    Player *player;

public:
    QuestForTrueColor()
    {
    }

    bool OnUserCreate() override
    {
        mapLayer = new Layer("Map", this);
        Map *map = new Map();
        mapLayer->AddNode(map);
        player = new Player(map);
        mapLayer->AddNode(player);

        mapLayer->OnCreate();
        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        fDeltaTime += fElapsedTime;
        Camera *camera = mapLayer->GetCamera();

        if (GetKey(olc::Key::ESCAPE).bPressed)
        {
            OnUserCreate();
            return true;
        }

        if (GetKey(olc::Key::TAB).bPressed)
        {
            DEBUG = !DEBUG;
        }

        float fPlayerPosX = player->GetPosition()->x;
        float fPlayerPosY = player->GetPosition()->y;
        float halfScreenWidth = ScreenWidth() / 2;
        float halfScreenHeight = ScreenHeight() / 2;

        olc::vf2d playerPos = {fPlayerPosX - halfScreenWidth, fPlayerPosY - halfScreenHeight};
        mapLayer->SetCameraPosition(playerPos);

        if (fDeltaTime >= 1.0f / TARGET_PHYSICS_PROCESS)
        {
            mapLayer->PhysicsProcess(fDeltaTime);
            fDeltaTime -= 1.0f / TARGET_PHYSICS_PROCESS;
        }

        mapLayer->Process(fElapsedTime);

        return true;
    }
};
