#pragma once

using namespace olc::utils::geom2d;

class Player : public Node
{
private:
    Animation *idle;
    Animation *walk;
    Map *map;

    olc::vf2d *position;
    olc::vf2d *velocity;
    olc::vf2d *acceleration;

    bool isFacingRight = true;
    bool canJump = false;

    ray<float> *downRay;
    ray<float> *facingRay;
    ray<float> *upRay;

public:
    Player(Map *map) : Node(), map(map)
    {
    }

    void OnCreate()
    {
        idle = new Animation(1.0f, {
                                       "assets/player/idle_0.png",
                                       "assets/player/idle_1.png",
                                   });
        walk = new Animation(0.5f, {
                                       "assets/player/idle_1.png",
                                       "assets/player/idle_0.png",
                                       "assets/player/walk_L.png",
                                       "assets/player/walk_L.png",
                                       "assets/player/idle_1.png",
                                       "assets/player/idle_0.png",
                                       "assets/player/walk_R.png",
                                       "assets/player/walk_R.png",
                                   });

        position = new olc::vf2d(10, 10);
        velocity = new olc::vf2d(0, 0);
        acceleration = new olc::vf2d(0, 0);

        downRay = new ray<float>();
        downRay->direction = {0, 1};

        facingRay = new ray<float>();
        facingRay->direction = {1, 0};

        upRay = new ray<float>();
        upRay->direction = {0, -1};
    }

    void OnPhysicsProcess(float fElapsedTime)
    {
        velocity->y += 500 * fElapsedTime;

        if (GetKey(olc::Key::UP).bHeld && !IsJumping() && canJump)
        {
            velocity->y = -200;
            canJump = false;
        }

        if (GetKey(olc::Key::RIGHT).bHeld)
        {
            velocity->x = 200;
            isFacingRight = true;
        }
        else if (GetKey(olc::Key::LEFT).bHeld)
        {
            velocity->x = -200;
            isFacingRight = false;
        }
        else
        {
            velocity->x = 0;
        }

        auto collisions = map->IsColliding(downRay, 16);
        if (collisions.size() > 0 && !IsJumping())
        {
            position->y = collisions[0].y - 32;
            velocity->y = 0;
        }

        if (position->y > GetEngine()->ScreenHeight() - 32)
        {
            position->y = GetEngine()->ScreenHeight() - 32;
            velocity->y = 0;
        }

        canJump = velocity->y == 0;

        facingRay->origin = *position;
        facingRay->origin.x += 16;
        facingRay->origin.y += 30;
        facingRay->direction.x = isFacingRight ? 1 : -1;

        downRay->origin = *position;
        downRay->origin.x += 16;
        downRay->origin.y += 32;

        upRay->origin = *position;
        upRay->origin.x += 16;
        upRay->origin.y += 0;

        auto faceColliders = map->IsColliding(facingRay, 16);
        if (faceColliders.size() > 0)
        {
            position->x = faceColliders[0].x - (isFacingRight ? 32 : 0);
            velocity->x = 0;
        }

        *velocity += *acceleration * fElapsedTime;
        *position += *velocity * fElapsedTime;
    }

    bool IsFacingRight()
    {
        return isFacingRight;
    }

    bool IsMoving()
    {
        return velocity->x != 0;
    }

    bool IsJumping()
    {
        return velocity->y < 0 && !canJump;
    }

    olc::Decal *GetAnimation(float fElapsedTime)
    {
        if (IsMoving())
        {
            return walk->GetFrame(fElapsedTime);
        }

        return idle->GetFrame(fElapsedTime);
    }

    void OnProcess(float fElapsedTime)
    {
        olc::vf2d scale = {1.0f, 1.0f};
        olc::vf2d position = *this->position;

        if (!isFacingRight)
        {
            scale.x = -1.0f;
            position.x += 32;
        }

        GetEngine()->DrawDecal(position, GetAnimation(fElapsedTime), scale);

        if (IsDebug())
        {
            GetEngine()->DrawLineDecal(downRay->origin, downRay->origin + downRay->direction * 32, olc::WHITE);
            GetEngine()->DrawLineDecal(facingRay->origin, facingRay->origin + facingRay->direction * 32, olc::WHITE);
            GetEngine()->DrawLineDecal(upRay->origin, upRay->origin + upRay->direction * 32, olc::WHITE);
        }
    }
};
