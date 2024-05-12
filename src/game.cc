
#define OLC_SOUNDWAVE
#define OLC_PGE_APPLICATION
#define OLC_IGNORE_VEC2D
#define TARGET_PHYSICS_PROCESS 60
#define USE_PIXEL_GAME_ENGINE
#define PREFER_DECAL
#define SPRITE_SIZE 32

#include <cassert>
#include <random>
#include <stack>
#include <iostream>
#include <olcUTIL_Geometry2D.h>
#include <olcPixelGameEngine.h>
#include <LDtkLoader/Project.hpp>
#include "core/audio.h"
#include "core/ui.h"
#include "core/nodes.h"
#include "registry.h"
#include "menu.cc"
#include "player.cc"
#include "npcs.cc"
#include "collectables.cc"
#include "minigames.cc"
#include "ui.cc"

CoreNode *CreateNode(GameNode *game, const ldtk::Entity &entity)
{
    auto type = entity.getName();
    return CoreNodeFactory::get().createNode(type, entity, game);
}

MiniGame *CreateMiniGame(const std::string &name, GameNode *game)
{
    return new ShellGame(game);
}

class QuestForTrueColor : public olc::PixelGameEngine
{
private:
    GameNode *gameNode = nullptr;
    MenuNode *menuNode = nullptr;
    olc::sound::WaveEngine soundEngine;
    Sound gameSound = Sound("assets/sfx/huperboloid.wav");
    bool paused = true;
    bool didSkipFrame = false;

    olc::HWButton upState;
    olc::HWButton downState;
    olc::HWButton leftState;
    olc::HWButton rightState;
    olc::HWButton enterState;
    olc::HWButton escapeState;
    olc::HWButton f1State;

public:
    QuestForTrueColor()
    {
    }

    bool OnConsoleCommand(const std::string &sCommand) override
    {
        // check if command is lvl <level_number>
        if (sCommand.find("lvl") == 0)
        {
            auto level = "level_" + sCommand.substr(4);
            gameNode->loadLevel(level);
            return true;
        }

        // Flip debug mode
        if (sCommand == "debug")
        {
            DEBUG = !DEBUG;
            return true;
        }

        // Add one life
        if (sCommand == "life")
        {
            auto *player = gameNode->getChild<PlayerNode>();
            player->addLife();
            player->addMoney(1);
            gameNode->isGameOver = false;
            return true;
        }

        if (sCommand.find("minigame") == 0)
        {
            auto minigame = sCommand.substr(9);
            gameNode->setMiniGame(minigame);
            return true;
        }

        return false;
    }

    bool OnUserCreate() override
    {
        SetContext(this);
        paused = true;

        auto *uiNode = new UINode(nullptr);
        menuNode = new MenuNode();
        gameNode = new GameNode(uiNode);
        uiNode->game = gameNode;

        menuNode->game = gameNode;
        gameNode->onCreated();
        menuNode->onCreated();

        soundEngine.InitialiseAudio();
        setSoundEngine(&soundEngine);

        return true;
    }

    bool OnUserUpdate(float fElapsedTime) override
    {
        if (fElapsedTime > 0.1f)
        {
            didSkipFrame = true;
            updateKeyStates();
            fElapsedTime = 0.0;
        }

        if (!didSkipFrame)
        {
            updateKeyStates();
        }

        if (f1State.bPressed)
        {
            ConsoleShow(olc::Key::ESCAPE, false);
        }
        else if (escapeState.bPressed && !IsConsoleShowing())
        {
            if (menuNode->canContinueGame())
            {
                paused = !paused;
                menuNode->selectedOption = "Continue";
            }
            else
            {
                if (!paused)
                {
                    paused = true;
                    menuNode->selectedOption = "New Game";
                }
            }
        }
        else if (paused)
        {
            soundEngine.StopAll();
            gameSound.SetPlayed(false);
            if (upState.bPressed)
            {
                menuNode->onUp();
            }
            else if (downState.bPressed)
            {
                menuNode->onDown();
            }
            else if (enterState.bPressed)
            {
                if (menuNode->selectedOption == "Exit")
                {
                    return false;
                }
                else if (menuNode->selectedOption == "New Game")
                {
                    menuNode->canContinue = true;
                    getGameNode(true);
                }

                paused = false;
            }

            menuNode->onUpdated(fElapsedTime);
        }
        else
        {
            auto *gameNode = getGameNode();
            gameSound.Play(true, false);

            if (upState.bHeld)
                gameNode->onUp();

            if (downState.bHeld)
                gameNode->onDown();

            if (leftState.bHeld)
                gameNode->onLeft();

            if (rightState.bHeld)
                gameNode->onRight();

            if (enterState.bPressed)
                gameNode->onEnter();

            gameNode->onUpdated(fElapsedTime);
        }

        didSkipFrame = false;
        return true;
    }

    bool OnUserDestroy() override
    {
        delete menuNode;
        delete gameNode;

        return true;
    }

    GameNode *getGameNode(bool recreate = false)
    {
        if (!gameNode)
        {
            return nullptr;
        }

        if (recreate)
        {
            gameNode->onCreated();
        }

        return gameNode;
    }

    void updateKeyStates()
    {
        if (!IsConsoleShowing())
        {
            upState = GetKey(olc::Key::UP);
            downState = GetKey(olc::Key::DOWN);
            leftState = GetKey(olc::Key::LEFT);
            rightState = GetKey(olc::Key::RIGHT);
            enterState = GetKey(olc::Key::SPACE);
        }
        escapeState = GetKey(olc::Key::ESCAPE);
        f1State = GetKey(olc::Key::F1);
    }
};
