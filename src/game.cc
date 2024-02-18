
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
    std::vector<Layer *> layers;
    float fDeltaTime = 0.0f;

public:
    QuestForTrueColor()
    {
        sAppName = "Quest for True Color";
        layers = std::vector<Layer *>();
    }

    bool OnUserCreate() override
    {
        // Adding all layers
        Layer *mapLayer = new Layer("Map", this);
        Map *map = new Map();

        mapLayer->AddNode(map);
        mapLayer->AddNode(new Player(map));

        layers.push_back(mapLayer);

        // Running onCreate for all layers
        for (auto layer : layers)
        {
            layer->OnCreate();
        }

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        fDeltaTime += fElapsedTime;

        if (GetKey(olc::Key::TAB).bPressed)
        {
            DEBUG = !DEBUG;
        }

        // Only process physics at a fixed rate
        // this prevents the game from running faster on faster hardware
        if (fDeltaTime >= 1.0f / TARGET_PHYSICS_PROCESS)
        {

            for (auto layer : layers)
            {
                layer->PhysicsProcess(fDeltaTime);
            }
            fDeltaTime = 0.0f;
        }

        // Process all layers
        for (auto layer : layers)
        {
            layer->Process(fElapsedTime);
        }
        return true;
    }
};
