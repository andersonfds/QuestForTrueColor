#include <olcPixelGameEngine.h>
#include <vector>
#include "characters/character.h"
#include "characters/animation.h"
#include "characters/collision.h"

enum LookDirection
{
    LEFT,
    RIGHT
};

enum LookVertical
{
    UP,
    DOWN
};

class player : public character
{
public:
    player() : character()
    {
        this->lives = 3;
        this->speed = 0;
        this->position = new olc::vf2d(20, 150);
        this->width = 32;
        this->height = 32;
        this->controllable = true;
        this->lookDirection = LookDirection::RIGHT;
        this->lookVertical = LookVertical::DOWN;

        this->idle = new animation({
                                       "assets/agnes/idle-0.png",
                                       "assets/agnes/idle-1.png",
                                   },
                                   0.5f);

        this->attack = new animation({
                                         "assets/agnes/attack-0.png",
                                         "assets/agnes/attack-1.png",
                                     },
                                     0.1f);
        this->attack->setLoop(false);

        this->walk = new animation({
                                       "assets/agnes/walk-l.png",
                                       "assets/agnes/walk-l.png",
                                       "assets/agnes/idle-0.png",
                                       "assets/agnes/idle-1.png",
                                       "assets/agnes/walk-r.png",
                                       "assets/agnes/walk-r.png",
                                       "assets/agnes/idle-0.png",
                                       "assets/agnes/idle-1.png",
                                   },
                                   0.08f);
    }

    void Render(olc::PixelGameEngine *engine, float fElapsedTime) override
    {
        this->speed = 0;

        if (this->controllable)
        {
            if (engine->GetKey(olc::Key::UP).bHeld && !this->isVerticalLocked(LookVertical::UP))
            {
                this->speed = 32;
                this->position->y -= this->speed * fElapsedTime;
                this->lookVertical = LookVertical::UP;
            }
            if (engine->GetKey(olc::Key::DOWN).bHeld && !this->isVerticalLocked(LookVertical::DOWN))
            {
                this->speed = 32;
                this->position->y += this->speed * fElapsedTime;
                this->lookVertical = LookVertical::DOWN;
            }
            if (engine->GetKey(olc::Key::LEFT).bHeld && !this->isDirectionLocked(LookDirection::LEFT))
            {
                this->speed = 32;
                this->position->x -= this->speed * fElapsedTime;
                this->lookDirection = LookDirection::LEFT;
            }
            if (engine->GetKey(olc::Key::RIGHT).bHeld && !this->isDirectionLocked(LookDirection::RIGHT))
            {
                this->speed = 32;
                this->position->x += this->speed * fElapsedTime;
                this->lookDirection = LookDirection::RIGHT;
            }

            if (engine->GetKey(olc::Key::SPACE).bPressed && !this->attacking)
            {
                this->startAttack();
            }

            olc::Sprite::Flip flip = olc::Sprite::Flip::NONE;
            if (this->lookDirection == LookDirection::LEFT)
            {
                flip = olc::Sprite::Flip::HORIZ;
            }

            engine->SetPixelMode(olc::Pixel::ALPHA);
            engine->DrawSprite(this->position->x, this->position->y, this->getCurrentAnimation()->getCurrentFrame(fElapsedTime), 1, flip);
            engine->SetPixelMode(olc::Pixel::NORMAL);

            // box *raycastBox = this->getRaycastBox();
            // engine->DrawRect(raycastBox->position->x, raycastBox->position->y, raycastBox->width, raycastBox->height, olc::GREEN);
            // box *raycastBoxVertical = this->getRaycastBoxVertical();
            // engine->DrawRect(raycastBoxVertical->position->x, raycastBoxVertical->position->y, raycastBoxVertical->width, raycastBoxVertical->height, olc::GREEN);
        }
    }

    box *getRaycastBox()
    {
        if (this->lookDirection == LookDirection::LEFT)
        {
            return new box(new olc::vf2d(this->position->x + 8, this->position->y + 5), 8, this->height - 10);
        }

        return new box(new olc::vf2d(this->position->x + this->width - 16, this->position->y + 5), 8, this->height - 10);
    }

    box *getRaycastBoxVertical()
    {
        if (this->lookVertical == LookVertical::UP)
        {
            return new box(new olc::vf2d(this->position->x + 10, this->position->y), this->width - 22, 8);
        }

        return new box(new olc::vf2d(this->position->x + 10, this->position->y + this->height - 8), this->width - 22, 8);
    }

    void OnVerticalCollision(bool colliding, box *collider)
    {

        if (!colliding || collider->transparent)
        {
            this->verticalLocked = nullptr;
            return;
        }

        this->verticalLocked = &this->lookVertical;
    }

    void OnHorizontalCollision(bool colliding, box *collider)
    {
        if (!colliding || collider->transparent)
        {
            this->directionLocked = nullptr;
            return;
        }

        this->directionLocked = &this->lookDirection;
    }

    bool isDirectionLocked(LookDirection direction)
    {
        return this->directionLocked != nullptr && *this->directionLocked == direction || this->attacking;
    }

    bool isVerticalLocked(LookVertical vertical)
    {
        return this->verticalLocked != nullptr && *this->verticalLocked == vertical || this->attacking;
    }

    void startAttack()
    {
        this->attacking = true;
    }

private:
    animation *idle;
    animation *walk;
    animation *attack;

    LookDirection lookDirection;
    LookVertical lookVertical;
    LookDirection *directionLocked;
    LookVertical *verticalLocked;
    bool attacking = false;

    animation *getCurrentAnimation()
    {
        if (this->attack->isFinished())
        {
            this->attacking = false;
            this->attack->reset();
        }

        if (this->attacking)
        {
            return this->attack;
        }
        if (this->speed == 0)
        {
            return this->idle;
        }
        return this->walk;
    }
};