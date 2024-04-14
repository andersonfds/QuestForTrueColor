
class PlayerNode : public EntityNode
{
private:
    olc::vf2d checkpoint;
    AnimatedAssetProvider *animations;
    olc::vf2d velocity = {0, 0};
    bool isOnGround = false;
    olc::utils::geom2d::rect<float> *playerCollider;
    bool lockRight = false;
    bool lockLeft = false;
    olc::vf2d acceleration;
    float jumpTime = 0.0f;
    bool isFacingRight = true;
    uint8_t lives = 3;
    uint8_t money = 0;
    uint8_t storage = 0;
    uint8_t selectedIndex = 0;
    CoreNode *child = nullptr;

public:
    PlayerNode(const ldtk::Entity &entity, GameNode *game) : EntityNode(entity, game)
    {
    }

    uint8_t getLives()
    {
        return lives;
    }

    uint8_t getMoney()
    {
        return money;
    }

    uint8_t getStorage()
    {
        return storage;
    }

    bool isStorageFull()
    {
        return children.size() >= storage;
    }

    uint8_t getSelectedIndex()
    {
        return selectedIndex;
    }

    bool isSelected(CoreNode *node)
    {
        return child == node;
    }

    void expandStorage(uint8_t amount)
    {
        if (storage + amount <= 9)
            storage += amount;
    }

    void addMoney(uint8_t amount)
    {
        money += amount;
    }

    ~PlayerNode()
    {
        delete playerCollider;
        delete animations;
    }

    bool addChild(CoreNode *node) override
    {
        if (EntityNode::addChild(node))
        {
            if (children.size() == 1)
            {
                selectedIndex = 0;
                child = node;
            }
            return true;
        }

        return false;
    }

    void onCreated() override
    {
        EntityNode::onCreated();

        // Animation definitions
        this->animations = new AnimatedAssetProvider(position, getSpriteDrawPosition(), {1, 1}, {SPRITE_SIZE, SPRITE_SIZE}, olc::WHITE);

        this->animations->AddAnimation("idle", 4.0, {{}, {1, 0}});
        this->animations->AddAnimation("walk", 30.0, {{}, {3, 0}, {3, 0}, {3, 0}, {}, {1, 0}, {2, 0}, {2, 0}, {1, 0}});
        this->animations->AddAnimation("jump", 4.0, {{4, 0}});
        this->animations->AddAnimation("fall", 4.0, {{5, 0}});
        this->animations->AddAnimation("dead", 4.0, {{6, 0}});

        this->animations->PlayAnimation("idle");

        camera->offset->x = SCREEN_WIDTH * 0.5;
        camera->offset->y = SCREEN_HEIGHT * 0.66 - 20;
        playerCollider = new olc::utils::geom2d::rect<float>(position, {SPRITE_SIZE, SPRITE_SIZE});
        checkpoint = position;
        lives = 3;
        storage = 1;
        money = 0;
    }

    void onUpdated(float fElapsedTime) override
    {
        if (camera->IsOfflimits(position))
        {
            onInstantDeath();
            return;
        }

        invokeItemFromStorage();

        acceleration.y += 9.8f * fElapsedTime;

        // limit vertical velocity for falling
        if (velocity.y > 600)
        {
            velocity.y = 600;
        }

        camera->position->x = position.x;
        camera->position->y = position.y;
        playerCollider->pos = position;

        if (velocity.y < 0)
        {
            jumpTime += fElapsedTime;
        }

        // Applying velocity
        velocity += acceleration;
        computeCollisions();
        position += velocity * fElapsedTime;

        // Updating the animations
        if (isOnGround)
        {
            if (velocity.x != 0)
            {
                animations->PlayAnimation("walk", false);
            }
            else
            {
                animations->PlayAnimation("idle", false);
            }
        }
        else
        {
            if (velocity.y < 0)
            {
                animations->PlayAnimation("jump", false);
            }
            else if (velocity.y > 0)
            {
                animations->PlayAnimation("fall", false);
            }
        }
        velocity.x = 0;

        // Rendering the player
        animations->Update(fElapsedTime);
        olc::vf2d drawPosition = {this->position.x, this->position.y};
        camera->WorldToScreen(drawPosition);
        auto *options = animations->GetAssetOptions();
        options->position = drawPosition;
        auto scaleFactor = isFacingRight ? 1 : -1;
        auto positionFactor = isFacingRight ? 0 : 1;
        options->scale.x = scaleFactor;
        options->position.x += positionFactor * options->size.x;

        Image(spritesProvider, options);
    }

    olc::vf2d getDirection()
    {
        return {isFacingRight ? 1.0f : -1.0f, 0.0f};
    }

    olc::utils::geom2d::rect<float> getCollider()
    {
        return *playerCollider;
    }

    void invokeItemFromStorage()
    {
        for (int i = 0; i < storage; i++)
        {
            auto key = static_cast<olc::Key>(olc::Key::K1 + i);
            auto hasChild = children.size() > i;
            if (Pressed(key))
            {
                selectedIndex = i;
                if (hasChild)
                    child = children[i];
                else
                    child = nullptr;
                break;
            }
        }
    }

    void onUp() override
    {
        EntityNode::onUp();
        if (isOnGround)
        {
            jumpTime = 0.0f;
            this->velocity.y = -250;
            isOnGround = false;
        }
        else if (jumpTime > 0)
        {
            this->acceleration.y *= 0.98;
        }
    }

    void onLeft() override
    {
        EntityNode::onLeft();
        isFacingRight = false;

        if (lockLeft)
        {
            return;
        }

        this->velocity.x = -100;
    }

    void onRight() override
    {
        EntityNode::onRight();
        isFacingRight = true;

        if (lockRight)
        {
            return;
        }

        this->velocity.x = 100;
    }

    void onDown() override
    {
        EntityNode::onDown();

        if (children.empty())
            return;

        auto *child = children[0];
        child->position = position;
        moveChildToRoot(child, game);
    }

    void onTakeDamage()
    {
        lives--;
        if (lives <= 0)
        {
            onInstantDeath();
        }
    }

    void onInstantDeath()
    {
        lives = 0;
        game->onGameOver();
    }

    void computeCollisions()
    {
        auto &colliders = this->game->getOnScreenColliders();

        isOnGround = false;
        lockRight = false;
        lockLeft = false;

        for (auto &collider : colliders)
        {
            if (!overlaps(*collider, *playerCollider))
            {
                continue;
            }

            bool isFalling = velocity.y > 0;
            bool isJumping = velocity.y < 0;
            bool isFacingRight = velocity.x > 0;

            float playerBottom = playerCollider->pos.y + playerCollider->size.y;
            float playerLeft = playerCollider->pos.x;
            float playerRight = playerCollider->pos.x + playerCollider->size.x;

            float colliderTop = collider->pos.y;
            float colliderLeft = collider->pos.x;
            float colliderRight = collider->pos.x + collider->size.x;
            float colliderBottom = collider->pos.y + collider->size.y;

            bool isFacingWallToTheRight = playerRight > colliderLeft && playerLeft < colliderLeft;
            bool isFacingWallToTheLeft = playerLeft < colliderRight && playerRight > colliderLeft;
            float factor = isFacingRight ? 1.0f : -1.0f;
            float tolerance = 5.0f;
            olc::Pixel color = olc::WHITE;

            if (isFalling && playerBottom - colliderTop < 5)
            {
                // Calculating the horizontal intersection size between the player and the collider
                // if is more than 10% of the player size we can consider a collision
                // We are checking if the player is at least 10% inside the collider horizontally
                float horizontalIntersection = std::min(playerRight, colliderRight) - std::max(playerLeft, colliderLeft);
                if (horizontalIntersection > playerCollider->size.x * 0.1)
                {
                    isOnGround = true;
                    velocity.y = 0;
                    acceleration.y = 0;
                    position.y = colliderTop - playerCollider->size.y;
                    color = olc::GREEN;
                }
            }
            else
            {
                if (isJumping && colliderBottom - playerCollider->pos.y < 5)
                {
                    // Calculating the horizontal intersection size between the player and the collider
                    // if is more than 10% of the player size we can consider a collision
                    // We are checking if the player is at least 10% inside the collider horizontally
                    float horizontalIntersection = std::min(playerRight, colliderRight) - std::max(playerLeft, colliderLeft);
                    if (horizontalIntersection > playerCollider->size.x * 0.1)
                    {
                        velocity.y = 0;
                        acceleration.y = 0;
                        position.y = colliderBottom;
                        color = olc::YELLOW;
                    }
                }

                /// Calculating the vertical intersection size between the player and the collider
                /// if is more than 10% of the player size we can consider a collision
                /// We are checking if the player is at least 10% inside the collider vertically
                float verticalIntersection = std::min(playerBottom, colliderBottom) - std::max(playerCollider->pos.y, collider->pos.y);
                float bottomThreshold = std::abs(playerBottom - colliderBottom);

                if (verticalIntersection > playerCollider->size.y * 0.05)
                {
                    if (isFacingWallToTheRight && !lockRight)
                    {
                        color = olc::RED;
                        lockRight = true;
                        float newPosition = colliderLeft - playerCollider->size.x;
                        // if the position is less than 5% different from the new position we update it
                        if (std::abs(position.x - newPosition) < tolerance)
                        {
                            position.x = newPosition;
                        }
                    }

                    else if (isFacingWallToTheLeft && !lockLeft)
                    {
                        color = olc::BLUE;
                        lockLeft = true;
                        float newPosition = colliderRight;
                        // if the position is less than 5% different from the new position we update it
                        if (std::abs(position.x - newPosition) < tolerance)
                        {
                            position.x = newPosition;
                        }
                    }
                }
            }
            if (DEBUG)
            {
                auto pos = collider->pos;
                this->camera->WorldToScreen(pos);
                Rect(pos, collider->size, color);
            }
        }
    }
};