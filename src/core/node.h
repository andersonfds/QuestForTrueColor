#pragma once

class GameNode
{
protected:
    olc::vf2d position;
    olc::vf2d size;

public:
    virtual void OnCreate() = 0;

    virtual void OnUpdate(float fElapsedTime) = 0;
};