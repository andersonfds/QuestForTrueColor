#include <olcPixelGameEngine.h>
#include <vector>
#include "characters/character.h"
#include "characters/animation.h"

class player : public character
{
public:
    player() : character()
    {
        this->lives = 3;
        this->speed = 0;
        this->position = new olc::vf2d(0, 0);
        this->width = 32;
        this->height = 32;
        this->controllable = true;

        this->idle = new animation({
                                       "assets/agnes/idle-0.png",
                                       "assets/agnes/idle-1.png",
                                   },
                                   0.5f);

        this->walk = new animation({
            "assets/agnes/walk-l.png",
            "assets/agnes/walk-l.png",
            "assets/agnes/idle-0.png",
            "assets/agnes/idle-1.png",
            "assets/agnes/walk-r.png",
            "assets/agnes/walk-r.png",
            "assets/agnes/idle-0.png",
            "assets/agnes/idle-1.png",
        }, 0.08f);
    }

    void Render(olc::PixelGameEngine *engine, float fElapsedTime) override
    {
        this->speed = 0;

        if (this->controllable)
        {
            if (engine->GetKey(olc::Key::UP).bHeld)
            {
                this->speed = 32;
                this->position->y -= this->speed * fElapsedTime;
            }
            if (engine->GetKey(olc::Key::DOWN).bHeld)
            {
                this->speed = 32;
                this->position->y += this->speed * fElapsedTime;
            }
            if (engine->GetKey(olc::Key::LEFT).bHeld)
            {
                this->speed = 32;
                this->position->x -= this->speed * fElapsedTime;
                this->flip = olc::Sprite::Flip::HORIZ;
            }
            if (engine->GetKey(olc::Key::RIGHT).bHeld)
            {
                this->speed = 32;
                this->position->x += this->speed * fElapsedTime;
                this->flip = olc::Sprite::Flip::NONE;
            }

            engine->SetPixelMode(olc::Pixel::ALPHA);
            engine->DrawSprite(this->position->x, this->position->y, this->getCurrentAnimation()->getCurrentFrame(fElapsedTime), 1, this->flip);
        }
    }

private:
    olc::Sprite::Flip flip = olc::Sprite::Flip::NONE;
    animation *idle;
    animation *walk;

    animation *getCurrentAnimation()
    {
        if (this->speed == 0)
        {
            return this->idle;
        }
        return this->walk;
    }
};