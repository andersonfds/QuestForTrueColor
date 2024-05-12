
#pragma region Collectable Base

class Collectable : public EntityNode
{
private:
    float deltaTime = 0.0f;

protected:
    AnimatedAssetProvider *assetProvider;
    bool didCollect = false;
    bool enableWiggling = true;
    bool autoCollect = false;
    PlayerNode *player;
    olc::utils::geom2d::rect<float> collider;
    std::string hintText = "Item";
    Dialog dialog;

public:
    Collectable(const ldtk::Entity &entity, GameNode *game) : EntityNode(entity, game)
    {
        auto assetDrawPosition = getSpriteDrawPosition();
        assetProvider = new AnimatedAssetProvider(position, assetDrawPosition, {1, 1}, {SPRITE_SIZE, SPRITE_SIZE}, olc::WHITE);
        thumbnail = new AssetOptions({}, assetDrawPosition, {1, 1}, {SPRITE_SIZE, SPRITE_SIZE});
    }

    ~Collectable() override
    {
        delete assetProvider;
    }

    void onCreated() override
    {
        EntityNode::onCreated();

        collider = olc::utils::geom2d::rect<float>(position, {SPRITE_SIZE, SPRITE_SIZE});
        deltaTime = rand() % 100 / 100.0f;
        dialog = {"Press SPACE to collect items", 3.0f};
        collider.pos = position;
    }

    void onUpdated(float fElapsedTime) override
    {
        EntityNode::onUpdated(fElapsedTime);
        didCollect = this->parent != nullptr;

        if (!camera->IsOnScreen(position))
        {
            return;
        }

        if (didCollect && player != nullptr && !player->isSelected(this))
        {
            onIsNotActive(fElapsedTime);
            return;
        }

        if (deltaTime > 1.0f)
        {
            deltaTime = 0.0f;
        }

        collider.pos = position;
        assetProvider->Update(fElapsedTime);
        deltaTime += fElapsedTime;

        auto drawPosition = this->position;
        camera->WorldToScreen(drawPosition);
        auto *options = assetProvider->GetAssetOptions();
        options->position = drawPosition;

        if (enableWiggling)
            options->position.y -= 5 * std::sin(2 * 3.14 * deltaTime);

        auto textSize = TextSize(hintText);
        auto hintPosition = options->position;
        hintPosition.y -= 30;
        hintPosition.x += SPRITE_SIZE * 0.5;
        hintPosition -= textSize * 0.5;

        if (!didCollect && isCollidingWithPlayer())
        {
            onCanCollect();

            if (!autoCollect)
            {
                if (!this->game->getFlag("KnowsHowToCollect"))
                {
                    this->game->addDialog(dialog);
                    this->game->setFlag("KnowsHowToCollect", true);
                }

                Text(hintText, olc::WHITE, hintPosition);
            }
        }

        Render(fElapsedTime);

        if (didCollect)
            onIsActive(fElapsedTime);
    }

    virtual void onIsNotActive(float fElapsedTime) {}

    void onEnter() override
    {

        if (didCollect)
        {
            return;
        }

        if (isCollidingWithPlayer())
        {
            bool isStorageFull = player->isStorageFull();

            if (isStorageFull)
            {
                this->game->addDialog({"Can't store any more items.\nDrop the current item by pressing arrow down.", 2.0f});
                return;
            }

            this->game->setFlag("KnowsHowToCollect", true);
            onCollected();
        }
    }

    void onReparent() override
    {
        if (!this->parent)
        {
            position.x = std::round(position.x / SPRITE_SIZE) * SPRITE_SIZE;
            position.y = std::round(position.y / SPRITE_SIZE) * SPRITE_SIZE;
            didCollect = false;
            if (!autoCollect)
                enableWiggling = true;
        }
    }

    bool isCollidingWithPlayer()
    {
        if (!player)
        {
            player = game->getChild<PlayerNode>();
        }

        if (!player)
        {
            return false;
        }

        return overlaps(collider, player->getCollider());
    }

    virtual bool canCollect()
    {
        return isCollidingWithPlayer() && !didCollect;
    }

    virtual void onIsActive(float fElapsedTime) {}

    virtual void Render(float fDeltaTime)
    {
        Image(spritesProvider, assetProvider->GetAssetOptions());

        if (DEBUG)
        {
            auto pos = collider.pos;
            this->camera->WorldToScreen(pos);
            Rect(pos, collider.size, olc::WHITE);
        }
    }

    virtual void onCollected()
    {
        if (didCollect)
        {
            return;
        }

        if (!autoCollect)
            enableWiggling = false;

        reparent(player);
    }

    virtual void onCanCollect()
    {
        if (autoCollect)
        {
            onCollected();
        }
    }
};

#pragma endregion

#pragma region Tiny Purse

class TinyPurseNode : public Collectable
{
public:
    TinyPurseNode(const ldtk::Entity &entity, GameNode *game) : Collectable(entity, game)
    {
        hintText = "Tiny Purse";
    }

    void onCollected() override
    {
        this->game->removeChild(this);
        this->player->expandStorage(2);
        this->game->addDialog({"Now you can store stuff in your tiny purse", 2.0f});
    }
};
REGISTER_NODE_TYPE(TinyPurseNode, "purse")

#pragma endregion

#pragma region Bug Spray

class Particle
{
public:
    olc::vf2d position;
    olc::vf2d velocity;
    float lifespan;

    Particle(olc::vf2d startPos, olc::vf2d startVel, float life) : position(startPos), velocity(startVel), lifespan(life) {}

    void Update(float fElapsedTime)
    {
        position += velocity * fElapsedTime;
        lifespan -= fElapsedTime;
    }
};

olc::vf2d rotateVector(const olc::vf2d &vec, float angle)
{
    float cosA = cos(angle);
    float sinA = sin(angle);
    return {vec.x * cosA - vec.y * sinA, vec.x * sinA + vec.y * cosA};
}

class BugSprayNode : public Collectable
{
private:
    std::vector<Particle> particles;
    const uint8_t PARTICLE_COUNT = 80;
    const uint8_t EMISSION_RATE = 40;
    const uint8_t EMISSION_PER_SETUP = 8;
    Sound *spraySfx;

    uint8_t emitted = 0;
    float deltaLastEmission = 0.0f;

public:
    BugSprayNode(const ldtk::Entity &entity, GameNode *game) : Collectable(entity, game)
    {
        hintText = "Bug Spray";
        particles.reserve(PARTICLE_COUNT);
    }

    void onCreated() override
    {
        Collectable::onCreated();
        spraySfx = new Sound("assets/sfx/spray.wav", 1);
    }

    void onCollected() override
    {
        Collectable::onCollected();
        if (!this->game->getFlag("KnowsHowToUseBugSpray"))
        {
            this->game->addDialog({"You can use the " + hintText + " by pressing X", 2.0f, false, false, 10});
            this->game->setFlag("KnowsHowToUseBugSpray", true);
        }

        particles.clear();

        for (int i = 0; i < PARTICLE_COUNT; i++)
            particles.emplace_back(olc::vf2d{0, 0}, olc::vf2d{0, 0}, 0.0f);
    }

    void setupParticles()
    {
        int setup = 0;

        for (auto &p : particles)
        {
            if (p.lifespan > 0.0f)
                continue;

            if (++emitted > EMISSION_RATE || ++setup > EMISSION_PER_SETUP)
                break;

            deltaLastEmission = 0.0f;

            float angle = ((rand() % 45) - 22.5f) * 3.14159f / 180.0f;
            auto direction = player->getDirection();
            olc::vf2d vel = rotateVector(direction, angle) * (float)(rand() % 100 + 50);
            p.position = this->position;
            p.lifespan = (rand() % 100) / 100.0f * 2.0f + 0.1f;
            p.velocity = vel;
        }
    }

    void onIsActive(float fElapsedTime) override
    {
        auto aliveParticles = renderParticles(fElapsedTime);

        if (game->isGameOver)
            return;

        bool isHoldingSpray = Held(olc::Key::X);

        if (isHoldingSpray)
        {
            deltaLastEmission += fElapsedTime;

            if (!spraySfx->IsPlaying())
                spraySfx->Play(false, true);
        }

        if (aliveParticles <= EMISSION_RATE && isHoldingSpray && deltaLastEmission >= 0.05f)
        {
            emitted -= aliveParticles;
            setupParticles();
        }
    }

    void onIsNotActive(float fElapsedTime) override
    {
        renderParticles(fElapsedTime);
    }

private:
    uint8_t renderParticles(float fElapsedTime)
    {
        int aliveParticles = 0;
        auto &worldOptions = game->getGameEnum("world");
        auto &sprayPos = worldOptions["spray"];
        auto &textureRect = sprayPos.getIconTextureRect();

        olc::vf2d sprayPosition = {static_cast<float>(textureRect.x), static_cast<float>(textureRect.y)};
        auto npcs = game->getOnScreenChildrenOfType<CoreNPC>();

        for (auto &particle : particles)
        {
            if (particle.lifespan <= 0.0f)
                continue;

            particle.Update(fElapsedTime);
            olc::vf2d position = particle.position;
            camera->WorldToScreen(position);

            aliveParticles++;

            auto particleCollider = olc::utils::geom2d::rect<float>(particle.position, {SPRITE_SIZE, SPRITE_SIZE});
            particleCollider.size *= 0.5f;

            for (auto npc : npcs)
                if (overlaps(npc->getCollider(), particleCollider))
                    npc->onDamage();

            AssetOptions options = AssetOptions(position, sprayPosition, {1, 1}, {SPRITE_SIZE, SPRITE_SIZE});
            options.position = position;

            options.offset.y = particle.lifespan > 0.6f ? 0 : options.size.y;

            Image(spritesProvider, &options);
        }

        return aliveParticles;
    }
};
REGISTER_NODE_TYPE(BugSprayNode, "bug_spray")

#pragma endregion

#pragma region Coin

class CoinNode : public Collectable
{
private:
    Sound *coinUpSfx;

public:
    CoinNode(const ldtk::Entity &entity, GameNode *game) : Collectable(entity, game)
    {
        hintText = "Coin";
        enableWiggling = false;
        autoCollect = true;
    }

    ~CoinNode() override
    {
        delete coinUpSfx;
    }

    void onCreated() override
    {
        Collectable::onCreated();
        assetProvider->AddAnimation("idle", 10.0, {{}, {1, 0}, {2, 0}, {3, 0}, {4, 0}});
        assetProvider->PlayAnimation("idle", true);

        // Randomize the initial frame
        float fRandomElapsedTime = rand() % 100 / 100.0f;
        assetProvider->Update(fRandomElapsedTime);

        // Loading sound effects
        coinUpSfx = new Sound("assets/sfx/coin_up.wav");
    }

    void onCollected() override
    {
        this->game->removeChild(this);
        this->player->addMoney(1);
        coinUpSfx->Play(false, true);
    }
};
REGISTER_NODE_TYPE(CoinNode, "coin")

#pragma endregion

#pragma region Flower

class FlowerNode : public Collectable
{
public:
    FlowerNode(const ldtk::Entity &entity, GameNode *game) : Collectable(entity, game)
    {
        hintText = "Unknown Flower";
    }
};
REGISTER_NODE_TYPE(FlowerNode, "flower")

#pragma endregion

#pragma region Portal

class PortalNode : public Collectable
{
private:
    std::string targetLevel;

public:
    PortalNode(const ldtk::Entity &entity, GameNode *game) : Collectable(entity, game)
    {
        hintText = "Portal";
        autoCollect = true;
        enableWiggling = false;
    }

    void onCreated() override
    {
        Collectable::onCreated();
        assetProvider->AddAnimation("spin", 6.0f, {{0, 0}, {1, 0}, {2, 0}});
        assetProvider->PlayAnimation("spin", true);

        bool isEnabled = entity.getField<ldtk::FieldType::Bool>("enabled").value_or(false);
        targetLevel = entity.getField<ldtk::FieldType::String>("level").value_or("");

        if (isEnabled)
            game->enableLevelPortal();
    }

    void onUpdated(float fElapsedTime) override
    {
        if (game->isLevelPortalEnabled())
            Collectable::onUpdated(fElapsedTime);
    }

    void onCollected() override
    {
        if (game->isLevelPortalEnabled())
        {
            game->loadLevel(targetLevel);
        }
    }
};

REGISTER_NODE_TYPE(PortalNode, "portal")

#pragma endregion Portal

#pragma region CheckPoint

class CheckPointNode : public Collectable
{
private:
    Sound *checkpointActivatedSfx;

public:
    CheckPointNode(const ldtk::Entity &entity, GameNode *game) : Collectable(entity, game)
    {
        hintText = "Check Point";
        autoCollect = true;
        enableWiggling = false;
    }

    ~CheckPointNode() override
    {
        delete checkpointActivatedSfx;
    }

    void onCreated() override
    {
        Collectable::onCreated();
        position.y += 10;
        assetProvider->AddAnimation("idle", 1.0, {{}});
        assetProvider->AddAnimation("collected", 1.0, {{1, 0}});
        assetProvider->PlayAnimation("idle", true);
        checkpointActivatedSfx = new Sound("assets/sfx/flag_put.wav", 1);
    }

    void onCollected() override
    {
        if (didCollect)
        {
            return;
        }

        checkpointActivatedSfx->Play(false, false);
        PlayerNode *player = game->getChild<PlayerNode>();
        player->setCheckpoint(position);
        assetProvider->PlayAnimation("collected");
        didCollect = true;
    }
};

REGISTER_NODE_TYPE(CheckPointNode, "checkpoint")

#pragma endregion CheckPoint

#pragma region Gem

class GemNode : public CoreNode
{
private:
    olc::vf2d iconCoords;

public:
    GemNode(GameNode *game) : CoreNode("Gem", game)
    {
    }

    void onCreated() override
    {
        CoreNode::onCreated();
        iconCoords = game->getPositionForEnumValue("world", "gem");
    }

    void onUpdated(float fElapsedTime) override
    {
        AssetOptions options = AssetOptions(position, iconCoords, {1, 1}, {SPRITE_SIZE, SPRITE_SIZE});
        Image(game->spritesProvider, &options);
    }
};

class GemCollectableNode : public Collectable
{
public:
    bool visible = false;

    GemCollectableNode(const ldtk::Entity &entity, GameNode *game) : Collectable(entity, game)
    {
        hintText = "Gem";
    }

    void onCreated() override
    {
        Collectable::onCreated();
        visible = false;
    }

    void onUpdated(float fElapsedTime) override
    {
        if (game->getFlag("showGem"))
            Collectable::onUpdated(fElapsedTime);
    }

    void onCollected() override
    {
        Collectable::onCollected();
        game->addDialog({"Sorry, this is all I got for now XD", 1.0f, true, true, 10});
        game->addDialog({"Thanks for playing, though!", 1.0f, true, true, 11});
    }
};
REGISTER_NODE_TYPE(GemCollectableNode, "gem")

#pragma endregion Gem

#pragma region Shell

class ShellNode : public CoreNode
{
private:
    olc::vf2d iconCoords;
    float deltaTime = 0.0f;
    olc::vf2d newPosition;
    bool isMoving = false;
    float displayShellTime = 0.0f;

public:
    ShellNode(GameNode *game) : CoreNode("Shell", game)
    {
    }

    void onCreated() override
    {
        CoreNode::onCreated();
        iconCoords = game->getPositionForEnumValue("world", "shell");
        deltaTime = 0;
        newPosition = position;
    }

    void display()
    {
        displayShellTime = 4.0f;
    }

    void onUpdated(float fElapsedTime) override
    {
        deltaTime += fElapsedTime;
        displayShellTime -= fElapsedTime;

        if (deltaTime > 1.0f)
        {
            deltaTime = 0.0f;
        }

        if (displayShellTime < 0)
        {
            displayShellTime = 0;
        }

        if (position != newPosition)
        {
            auto diff = newPosition - position;
            auto speed = 100.0f;
            auto offset = speed * fElapsedTime;
            auto length = diff.mag();
            if (length < offset)
            {
                position = newPosition;
            }
            else
            {
                position += diff.norm() * offset;
            }
        }

        isMoving = position != newPosition;

        for (auto child : children)
            child->position = position;

        CoreNode::onUpdated(fElapsedTime);
        AssetOptions options = AssetOptions(position, iconCoords, {1, 1}, {SPRITE_SIZE, SPRITE_SIZE});

        if (displayShellTime > 0)
        {
            options.position.y -= 40 * displayShellTime;
        }
        else
        {
            options.position.y = position.y;
        }

        Image(game->spritesProvider, &options);
    }

    void moveTo(olc::vf2d position)
    {
        if (displayShellTime > 0)
        {
            return;
        }

        deltaTime = 0.0f;
        newPosition = position;
        isMoving = true;
    }

    void setPosition(olc::vf2d pos)
    {
        position = pos;
        newPosition = pos;
        isMoving = false;
    }

    bool getIsMoving()
    {
        return isMoving;
    }
};

#pragma endregion Shell
