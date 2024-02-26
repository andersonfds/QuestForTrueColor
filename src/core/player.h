#pragma once

using namespace olc::utils::geom2d;

class Player : public Node
{
private:
    AnimationController *idle;
    AnimationController *walk;
    AnimationController *jump;
    AnimationController *fall;

    olc::vf2d *position;
    olc::vf2d *velocity;
    olc::vf2d *acceleration;

    bool isFacingRight = true;
    bool canJump = false;
    bool isJumping = false;

    int width = 18;
    int height = 32;

    int money = 0;

    rect<float> *collider;
    ray<float> *downRay;
    ray<float> *facingRay;
    ray<float> *upRay;

public:
    void OnCreate()
    {
        idle = new AnimationController(this, 0.4f, 0, {0, 1});
        walk = new AnimationController(this, 0.1f, 0, {0, 3, 3, 0, 1, 2, 2, 1});
        jump = new AnimationController(this, 0.1f, 0, {4, 4});
        fall = new AnimationController(this, 0.1f, 0, {5, 5});

        Map *map = GetLayer()->GetNode<Map>();
        const auto &playerEntity = map->GetEntity(this->GetEntityID());
        auto initialPosition = playerEntity.getPosition();

        position = new olc::vf2d({static_cast<float>(initialPosition.x), static_cast<float>(initialPosition.y)});
        velocity = new olc::vf2d(0, 0);
        acceleration = new olc::vf2d(0, 0);
        GetCamera()->position = position;
        olc::vf2d offset = {GetEngine()->ScreenWidth() / 2.0f, GetEngine()->ScreenHeight() / 3.0f * 2 - 20};
        GetCamera()->offset->x = offset.x;
        GetCamera()->offset->y = offset.y;

        downRay = new ray<float>();
        downRay->direction = {0, 1};

        facingRay = new ray<float>();
        facingRay->direction = {1, 0};

        upRay = new ray<float>();
        upRay->direction = {0, -1};

        collider = new rect<float>({0, 0}, {static_cast<float>(width), static_cast<float>(height)});
    }

    void OnPhysicsProcess(float fElapsedTime)
    {
        acceleration->y += 12.0f * fElapsedTime;
        acceleration->x = 0;

        velocity->x = 0;
        velocity->y += acceleration->y;
        Map *map = GetLayer()->GetNode<Map>();

        if (std::abs(velocity->x) < 0.1f)
        {
            velocity->x = 0;
        }

        if (std::abs(velocity->y) < 0.1f)
        {
            velocity->y = 0;
        }

        auto downRayIntersections = map->IsColliding(downRay, 18);
        canJump = downRayIntersections.size() > 0;

        if (canJump && velocity->y > 0)
        {
            velocity->y = 0;
            position->y = downRayIntersections[0].y - 32;
            acceleration->y = 0;
        }

        if (GetEngine()->GetKey(olc::Key::UP).bHeld && canJump)
        {
            velocity->y = -200;
            canJump = false;
            isJumping = true;
        }

        auto facingRayIntersections = map->IsColliding(facingRay, width / 2);
        bool isFacingWall = facingRayIntersections.size() > 0;

        if (GetEngine()->GetKey(olc::Key::LEFT).bHeld)
        {
            if (!isFacingWall)
            {
                velocity->x = -100;
            }
            isFacingRight = false;
        }
        else if (GetEngine()->GetKey(olc::Key::RIGHT).bHeld)
        {
            if (!isFacingWall)
            {
                velocity->x = 100;
            }
            isFacingRight = true;
        }

        *position += *velocity * fElapsedTime;
        downRay->origin = *position;
        downRay->origin.x += 16;
        downRay->origin.y += 16;
        facingRay->origin = *position;
        facingRay->origin.x += 16;
        facingRay->origin.y += 16;
        facingRay->direction = isFacingRight ? olc::vf2d(1, 0) : olc::vf2d(-1, 0);
        collider->pos = *position;
    }

    bool IsFacingRight()
    {
        return isFacingRight;
    }

    bool IsOnGround()
    {
        return velocity->y >= 0;
    }

    bool IsMoving()
    {
        return velocity->x != 0;
    }

    bool IsJumping()
    {
        return velocity->y < 0 && !canJump;
    }

    AnimationController *GetAnimation(float fElapsedTime)
    {

        if (IsJumping())
        {
            return jump;
        }

        if (!canJump)
        {
            return fall;
        }

        if (IsMoving())
        {
            return walk;
        }

        return idle;
    }

    void OnProcess(float fElapsedTime)
    {
        olc::vf2d scale = {1.0f, 1.0f};

        Map *map = GetLayer()->GetNode<Map>();
        auto *animation = GetAnimation(fElapsedTime);

        Camera *camera = GetCamera();

        olc::vf2d drawPos = *this->position;

        if (!isFacingRight)
        {
            scale.x = -1.0f;
            drawPos.x += 32;
        }

        auto frame = animation->GetFrame(fElapsedTime);
        map->DrawTile(drawPos, animation->tilesetId, frame, {32.0f, 32.0f}, scale);

        if (IsDebug())
        {
            int offsetX = (32 - this->width) / 2;
            int offsetY = (32 - this->height) / 2;

            olc::vf2d pos = *this->position;
            pos.x += offsetX;
            pos.y += offsetY;

            camera->WorldToScreen(pos);
            GetEngine()->DrawRectDecal(pos, {static_cast<float>(this->width), static_cast<float>(this->height)}, olc::RED);

            // draw facing ray
            olc::vf2d facingRayOrigin = facingRay->origin;
            olc::vf2d facingRayDirection = facingRay->direction;

            camera->WorldToScreen(facingRayOrigin);

            facingRayDirection *= 32;
            facingRayDirection += facingRayOrigin;

            GetEngine()->DrawLineDecal(facingRayOrigin, facingRayDirection, olc::YELLOW);

            // draw down ray
            olc::vf2d downRayOrigin = downRay->origin;
            olc::vf2d downRayDirection = downRay->direction;

            camera->WorldToScreen(downRayOrigin);

            downRayDirection *= 5;
            downRayDirection += downRayOrigin;

            GetEngine()->DrawLineDecal(downRayOrigin, downRayDirection, olc::YELLOW);
        }
    }

    rect<float> *GetCollider()
    {
        return collider;
    }

    void AddMoney(int amount)
    {
        money += amount;
    }

    olc::vf2d *GetPosition()
    {
        return this->position;
    }
};