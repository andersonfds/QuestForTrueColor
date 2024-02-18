#pragma once

class Quest
{
public:
    Quest(olc::PixelGameEngine *pge)
    {
        this->pge = pge;
    }

private:
    olc::PixelGameEngine *pge;
};
