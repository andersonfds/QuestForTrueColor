#ifndef CHARACTER_H
#define CHARACTER_H

class character
{

public:
    virtual void Render(olc::PixelGameEngine *engine, float fElapsedTime) = 0;

    void SetControllable(bool enable)
    {
        this->controllable = enable;
    }

protected:
    int lives, speed, width, height;
    olc::vf2d *position;
    bool controllable;
    bool render = true;
};

#endif // CHARACTER_H