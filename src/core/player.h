#pragma once

using namespace olc::utils::geom2d;

class Item : public Node
{
public:
    olc::vf2d *position;
    olc::vf2d *framePosition;
    bool render;
    int tilesetId;

public:
    void OnCreate() override
    {
        Map *map = GetLayer()->GetNode<Map>();
        const auto &itemEntity = map->GetEntity(this->GetEntityID());
        auto position = itemEntity.getPosition();

        this->tilesetId = map->GetTilesetIDByPath(itemEntity.getTexturePath());
        this->position = new olc::vf2d({static_cast<float>(position.x), static_cast<float>(position.y)});

        auto texture = itemEntity.getTextureRect();
        this->framePosition = new olc::vf2d({static_cast<float>(texture.x), static_cast<float>(texture.y)});

        render = true;
        OnEntityDefined(itemEntity);
    }

    virtual void OnEntityDefined(const ldtk::Entity &entity) {}

    virtual void OnScreen(float fElapsedTime) {}

    virtual void OnOffScreen(float fElapsedTime) {}

    virtual void OnActivate() {}

    virtual void OnDeactivate() {}

    virtual void OnInteract(float fElapsedTime) {}

    virtual void OnExitInteract() {}

    virtual void CanEnableOther(Item *item) {}

    void OnProcess(float fElapsedTime) override
    {
        Camera *camera = GetCamera();
        if (!camera->IsOnScreen(*position))
        {
            return;
        }

        if (DEBUG)
        {
            olc::vf2d pos = *position;
            camera->WorldToScreen(pos);
            GetEngine()->DrawRectDecal(pos, {32, 32}, olc::BLUE);
        }

        OnScreen(fElapsedTime);
    }

    rect<float> GetCollider()
    {
        return rect<float>(*position, {32.0f, 32.0f});
    }
};

class Player : public Node
{
private:
    AnimationController *idle;
    AnimationController *walk;
    AnimationController *jump;
    AnimationController *fall;
    AnimationController *dead;

    Sound *gameOverSound;
    Sound *damageSound;
    Sound *coinUpSound;
    Sound *coinDownSound;
    Sound *liveUpSound;
    Sound *flagPutSound;

    bool hasFlower = false;
    Item *canActivateItem;
    olc::vf2d *lastCheckpoint;

    olc::vf2d *position;
    olc::vf2d *velocity;
    olc::vf2d *acceleration;

    bool isFacingRight = true;
    bool canJump = false;
    bool isJumping = false;
    bool isTryingToMove = false;
    bool enableControls = true;
    bool didActivateItem = false;
    bool canGoToNextLevel = false;

    int width = 18;
    int height = 32;

    int money = 0;
    int lives;

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
        dead = new AnimationController(this, 0.1f, 0, {6, 6});
        hasFlower = false;

        Map *map = GetLayer()->GetNode<Map>();

        const auto &playerEntity = map->GetEntity(this->GetEntityID());
        if (lastCheckpoint == nullptr)
        {
            auto initialPosition = playerEntity.getPosition();
            lastCheckpoint = new olc::vf2d({static_cast<float>(initialPosition.x), static_cast<float>(initialPosition.y)});
        }

        position = new olc::vf2d(*lastCheckpoint);

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

        collider = new rect<float>(*position, {static_cast<float>(width), static_cast<float>(height)});
        lives = 3;
        money = 0;

        gameOverSound = new Sound("assets/sfx/game_over.wav", 1);
        damageSound = new Sound("assets/sfx/damage.wav", 2);
        coinUpSound = new Sound("assets/sfx/coin_up.wav", 1);
        coinDownSound = new Sound("assets/sfx/coin_down.wav", 1);
        liveUpSound = new Sound("assets/sfx/live_up.wav", 1);
        flagPutSound = new Sound("assets/sfx/flag_put.wav", 3);
        LoadMusic("assets/sfx/huperboloid.wav", 0.1f);
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

        auto downRayIntersections = map->IsColliding(downRay, height / 2);
        canJump = downRayIntersections.size() > 0;

        if (canJump && velocity->y > 0)
        {
            velocity->y = 0;
            position->y = downRayIntersections[0].y - 32;
            acceleration->y = 0;
        }

        if (GetEngine()->GetKey(olc::Key::UP).bHeld && canJump && enableControls)
        {
            velocity->y = -200;
            canJump = false;
            isJumping = true;
        }

        auto facingRayIntersections = map->IsColliding(facingRay, width / 2);
        bool isFacingWall = facingRayIntersections.size() > 0;

        if (GetEngine()->GetKey(olc::Key::LEFT).bHeld && enableControls)
        {
            if (!isFacingWall)
            {
                velocity->x = -100;
            }
            isFacingRight = false;
            isTryingToMove = true;
        }
        else if (GetEngine()->GetKey(olc::Key::RIGHT).bHeld && enableControls)
        {
            if (!isFacingWall)
            {
                velocity->x = 100;
            }
            isFacingRight = true;
            isTryingToMove = true;
        }
        else
        {
            isTryingToMove = false;
        }

        if (didActivateItem && canActivateItem != nullptr && enableControls && GetEngine()->GetKey(olc::Key::Z).bHeld)
        {
            canActivateItem->OnInteract(fElapsedTime);
        }
        else if (canActivateItem != nullptr)
        {
            canActivateItem->OnExitInteract();
        }

        *position += *velocity * fElapsedTime;
        downRay->origin = *position;
        downRay->origin.x += 16;
        downRay->origin.y += 16;
        facingRay->origin = *position;
        facingRay->origin.x += 16;
        facingRay->origin.y += 30;
        facingRay->direction = isFacingRight ? olc::vf2d(1, 0) : olc::vf2d(-1, 0);
        collider->pos = *position + olc::vf2d(width / 2, 0);
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
        if (lives <= 0)
        {
            return dead;
        }

        if (IsJumping())
        {
            return jump;
        }

        if (!canJump)
        {
            return fall;
        }

        if (isTryingToMove)
        {
            return walk;
        }

        return idle;
    }

    void CanActivate(Item *item)
    {
        if (canActivateItem == nullptr)
        {
            canActivateItem = item;
            didActivateItem = false;
        }
        else if (canActivateItem != item)
        {
            canActivateItem->CanEnableOther(item);
        }
    }

    void ExitItem(Item *item)
    {
        if (canActivateItem == item)
        {
            canActivateItem = nullptr;
            didActivateItem = false;
        }
    }

    void OnProcess(float fElapsedTime)
    {
        olc::vf2d scale = {1.0f, 1.0f};

        Map *map = GetLayer()->GetNode<Map>();
        auto *animation = GetAnimation(fElapsedTime);

        Camera *camera = GetCamera();

        if (!camera->IsOnScreen(*position))
        {
            TakeDamage(1);
        }

        olc::vf2d drawPos = *this->position;

        if (!isFacingRight)
        {
            scale.x = -1.0f;
            drawPos.x += 32;
        }

        auto frame = animation->GetFrame(fElapsedTime);
        map->DrawTile(drawPos, animation->tilesetId, frame, {32.0f, 32.0f}, scale);

        if (canActivateItem != nullptr && GetEngine()->GetKey(olc::Key::SPACE).bPressed && enableControls && !map->HasDialogs())
        {
            didActivateItem = !didActivateItem;

            if (didActivateItem)
            {
                canActivateItem->OnActivate();
            }
            else
            {
                canActivateItem->OnDeactivate();
                canActivateItem = nullptr;
            }
        }

        if (IsDebug())
        {
            olc::vf2d pos = collider->pos;
            camera->WorldToScreen(pos);
            GetEngine()->DrawRectDecal(pos, collider->size, olc::RED);

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

        if (lives <= 0)
        {
            GetEngine()->DrawStringDecal({10, 10}, "Game Over", olc::WHITE, {2, 2});
            GetEngine()->DrawStringDecal({10, 50}, "Press ESC to restart", olc::WHITE, {2, 2});
            gameOverSound->Play();
            enableControls = false;
        }

        if (map->getEnableUI())
        {
            // Draw money
            olc::vf2d initialPos = {10.0f, GetEngine()->ScreenHeight() - 32.0f};
            map->DrawIcon(initialPos, {0, 0});
            GetEngine()->DrawStringDecal(initialPos + olc::vf2d(32, 0), std::to_string(money), olc::WHITE, {2, 2});

            // Draw lives
            for (int i = 0; i < lives; i++)
            {
                olc::vf2d initialPos = {-10 + GetEngine()->ScreenWidth() - 16.0f * (i + 1), GetEngine()->ScreenHeight() - 32.0f};
                map->DrawIcon(initialPos, {1, 0});
            }
        }
    }

    void OnTeleportedToAnotherLevel()
    {
    }

    rect<float> *GetCollider()
    {
        return collider;
    }

    void AddMoney(int amount)
    {
        money += amount;

        if (money >= 10)
        {
            money = 0;
            lives++;
            liveUpSound->Play(false, true);
        }
        else
        {
            coinUpSound->Play(false, true);
        }
    }

    void TakeDamage(int amount)
    {
        if (money <= 0)
        {
            lives -= amount;
            damageSound->Play(false, true);

            if (lives <= 0)
            {
                GetLayer()->GetEngine()->DrawStringDecal({10, 10}, "Game Over", olc::WHITE, {2, 2});
                GetLayer()->GetEngine()->DrawStringDecal({10, 50}, "Press ESC to restart", olc::WHITE, {2, 2});
                enableControls = false;
                return;
            }
            else
            {
                if (canActivateItem != nullptr)
                {
                    canActivateItem->OnDeactivate();
                }
                if (lastCheckpoint != nullptr)
                    *position = *lastCheckpoint;
            }
        }
        else
        {
            money = 0;
            coinDownSound->Play(false, true);
        }
    }

    void SetCheckpoint(olc::vf2d checkpoint)
    {
        if (*lastCheckpoint != checkpoint)
        {
            *lastCheckpoint = checkpoint;
            flagPutSound->Play(false, true);
        }
    }

    bool IsDead()
    {
        return lives <= 0;
    }

    olc::vf2d *GetPosition()
    {
        return this->position;
    }

    void DisableControls()
    {
        enableControls = false;
    }

    void EnableControls()
    {
        enableControls = true;
    }

    void SetHasFlower(bool hasFlower)
    {
        this->hasFlower = hasFlower;
    }

    bool HasFlower()
    {
        return hasFlower;
    }

    void SetCanGoToNextLevel(bool canGoToNextLevel)
    {
        this->canGoToNextLevel = canGoToNextLevel;
    }

    bool CanGoToNextLevel()
    {
        return canGoToNextLevel;
    }

    void ClearCheckpoints()
    {
        lastCheckpoint = nullptr;
    }
};
