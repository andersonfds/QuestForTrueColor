#pragma region NPCs

using namespace olc::utils::geom2d;

class CoreNPC : public EntityNode
{
protected:
    AnimatedAssetProvider *animProvider;
    bool isPlayingChat = false;

public:
    CoreNPC(const ldtk::Entity &entity, GameNode *game) : EntityNode(entity, game)
    {
    }

    ~CoreNPC()
    {
        delete animProvider;
    }

    void onCreated() override
    {
        EntityNode::onCreated();

        // NPCs dont have physics, so we need to adjust their position
        position.y += 8;

        animProvider = new AnimatedAssetProvider(position, getSpriteDrawPosition());
    }

    void onUpdated(float fElapsedTime) override
    {
        EntityNode::onUpdated(fElapsedTime);

        // Not spending resources on NPCs that are not on screen
        if (!camera->IsOnScreen(position))
            return;

        if (isPlayingChat && !game->hasPersistentDialogShowing())
        {
            isPlayingChat = false;
            onChatEnded();
        }

        onScreen(fElapsedTime);
    }

    bool collidesWithPlayer()
    {
        auto *player = game->getChild<PlayerNode>();
        auto collider = rect<float>(position, {SPRITE_SIZE, SPRITE_SIZE});
        return overlaps(collider, player->getCollider());
    }

    void onEnter() override
    {
        if (!camera->IsOnScreen(position))
            return;

        if (collidesWithPlayer())
            onInteracted(game->getChild<PlayerNode>());
    }

    virtual void onInteracted(PlayerNode *player) {}

    virtual void onChatEnded()
    {
    }

    virtual void onDamage() {}

    virtual void onScreen(float fElapsedTime)
    {
        auto drawPosition = this->position;
        camera->WorldToScreen(drawPosition);
        animProvider->Update(fElapsedTime);
        auto *options = animProvider->GetAssetOptions();
        options->position = drawPosition;
        Image(spritesProvider, options);
    }

    void chat(const std::vector<std::string> msgs)
    {
        if (isPlayingChat)
        {
            return;
        }

        isPlayingChat = true;
        uint8_t dialogId = 10;

        for (auto &msg : msgs)
            game->addDialog({msg, 0.0f, false, true, dialogId++});
    }
};

#pragma endregion NPCs

#pragma region Anderson - NPC

class FlowerNode;

enum class CurrentChat
{
    FLOWER,
    TURN_TO_VILLAIN,
    TURN_TO_VILLAIN_COMPLETE,
    NONE,
};

class AndersonNPC : public CoreNPC
{
private:
    uint8_t flowerDialogId = 10;
    CurrentChat currentChat = CurrentChat::NONE;
    bool isVillain = false;
    float animTurnToVillainDeltaTime = 0.0f;

public:
    AndersonNPC(const ldtk::Entity &entity, GameNode *game) : CoreNPC(entity, game)
    {
    }

    void onCreated() override
    {
        CoreNPC::onCreated();
        animProvider->AddAnimation("idle", 2.0f, {{}, {1, 0}});
        animProvider->AddAnimation("villainIdle", 1.0f, {{2, 0}});
        animProvider->PlayAnimation("idle");
        currentChat = CurrentChat::NONE;
        isVillain = false;
    }

    void onInteracted(PlayerNode *player) override
    {
        auto *flower = player->getChildOfType<FlowerNode>();

        if (!flower || isVillain)
        {
            return;
        }

        currentChat = CurrentChat::FLOWER;
    }

    void onChatEnded() override
    {
        switch (currentChat)
        {
        case CurrentChat::FLOWER:
            currentChat = CurrentChat::TURN_TO_VILLAIN;
            break;

        case CurrentChat::TURN_TO_VILLAIN:
            game->addDialog({"ACT 1: The villain", 4.0f, true, false});
            currentChat = CurrentChat::TURN_TO_VILLAIN_COMPLETE;
            animProvider->PlayAnimation("villainIdle");
            isVillain = true;
            break;

        case CurrentChat::TURN_TO_VILLAIN_COMPLETE:
            currentChat = CurrentChat::NONE;
            break;

        case CurrentChat::NONE:
            break;
        }
    }

    void onScreen(float fElapsedTime) override
    {
        CoreNPC::onScreen(fElapsedTime);

        switch (currentChat)
        {
        case CurrentChat::FLOWER:
            playSmellFlowerDialog();
            break;

        case CurrentChat::TURN_TO_VILLAIN:
            playTurnToVillainDialog();
            break;

        case CurrentChat::TURN_TO_VILLAIN_COMPLETE:
            playTurnToVillainCompleteDialog();
            break;

        case CurrentChat::NONE:
        default:
            break;
        }

        if (isVillain && currentChat == CurrentChat::NONE)
        {
            animTurnToVillainDeltaTime += fElapsedTime;

            if (animTurnToVillainDeltaTime > 1.0f)
                position.y -= 200 * fElapsedTime;
        }
    }

private:
    void playSmellFlowerDialog()
    {
        chat({
            "ANDERSON: Hello, I'm Anderson. I like cookies",
            "ANDERSON: Oh, you have a blue flower, can I smell it?",
            "YOU: Actually, it's purple",
            "ANDERSON: Bruh... Anyway, can I smell it?",
            "YOU: Sure, go ahead",
        });
    }

    void playTurnToVillainDialog()
    {
        chat({
            "ANDERSON: I Think the flower is making me feel weird",
            "ANDERSON: I'm turning into a villain",
        });
    }

    void playTurnToVillainCompleteDialog()
    {
        chat({
            "ANDERSON: I'm a villain now, I will destroy the world",
        });
        game->enableLevelPortal();
    }
};

REGISTER_NODE_TYPE(AndersonNPC, "anderson");

#pragma endregion Anderson - NPC

#pragma region Enemy - BEE

class BeeEnemy : public CoreNPC
{
private:
    olc::vf2d travelTo;
    olc::vf2d initialPosition;
    float speed = 100.0f;
    float delta = 0.0f;
    int direction = 0;
    bool harmless = false;

public:
    BeeEnemy(const ldtk::Entity &entity, GameNode *game) : CoreNPC(entity, game)
    {
    }

    bool isHarmless()
    {
        return harmless;
    }

    void onCreated() override
    {
        CoreNPC::onCreated();
        animProvider->setTint(olc::CYAN);
        animProvider->AddAnimation("idle", 4.9f, {{}, {1, 0}, {}, {1, 0}});
        animProvider->PlayAnimation("idle");
        animProvider->Update(0.5f + (rand() % 10) / 10.0f);
        auto initialPosition = entity.getPosition();
        const auto &travelToField = entity.getField<ldtk::FieldType::Point>("travel");
        position = olc::vf2d({static_cast<float>(initialPosition.x), static_cast<float>(initialPosition.y)});
        this->initialPosition = position;

        // get property isMad
        const auto &isMadField = entity.getField<ldtk::FieldType::Bool>("is_mad");
        harmless = isMadField.is_null() || !isMadField.value();

        if (!travelToField.is_null())
        {
            ldtk::IntPoint travelTo = travelToField.value();
            this->travelTo = olc::vf2d({static_cast<float>(travelTo.x), static_cast<float>(travelTo.y)});
            this->travelTo *= SPRITE_SIZE;
        }

        // if travel to is greater than the initial position, we need to move to the right
        if (this->travelTo.x > initialPosition.x)
        {
            direction = 1;
        }
        else
        {
            direction = -1;
        }
    }

    void onDamage() override
    {
        animProvider->setTint(olc::WHITE);
        harmless = true;
    }

    void computeDamageToPlayer()
    {
        if (harmless)
            return;

        bool isColliding = collidesWithPlayer();

        if (isColliding)
        {
            PlayerNode *player = game->getChild<PlayerNode>();
            player->onTakeDamage();
        }
    }

    olc::utils::geom2d::rect<float> getCollider() override
    {
        auto collider = olc::utils::geom2d::rect<float>(position, {SPRITE_SIZE, SPRITE_SIZE});

        if (direction < 0)
        {
            collider.pos.x -= SPRITE_SIZE;
        }

        // bees are slightly smaller than the sprite
        collider.size.x -= 10;
        collider.pos.x += 5;
        collider.size.y -= 10;
        collider.pos.y += 5;

        return collider;
    }

    void onScreen(float fElapsedTime) override
    {
        CoreNPC::onScreen(fElapsedTime);

        delta += fElapsedTime;
        position = initialPosition + (travelTo - initialPosition) * delta;

        if (delta > 1.0f)
        {
            delta = 0.0f;
            auto temp = initialPosition;
            initialPosition = travelTo;
            travelTo = temp;
            direction = -direction;
        }

        // flipping the sprite based on the direction
        animProvider->GetAssetOptions()->scale.x = direction;

        // if direction is negative we need to adjust the position by adding the sprite size
        if (direction < 0)
        {
            position.x += SPRITE_SIZE;
        }

        // Wiggling the bee vertically based on the delta
        position.y += sin(delta * 2 * 3.14) * 10;

        computeDamageToPlayer();
    }
};
REGISTER_NODE_TYPE(BeeEnemy, "bee");

#pragma endregion Enemy - BEE

#pragma region Erik - NPC

enum class ErikCurrentChat
{
    HELLO,
    THANKS,
    NONE,
};

class ErikNPC : public CoreNPC
{
private:
    ErikCurrentChat currentChat = ErikCurrentChat::NONE;
    bool didShowSecret = false;
    bool didClearBees = false;

public:
    ErikNPC(const ldtk::Entity &entity, GameNode *game) : CoreNPC(entity, game)
    {
    }

    void onCreated() override
    {
        CoreNPC::onCreated();
        animProvider->AddAnimation("idle", 2.0f, {{}, {1, 0}});
        animProvider->PlayAnimation("idle");
    }

    void onInteracted(PlayerNode *player) override
    {
        if (didClearBees)
        {
            currentChat = ErikCurrentChat::THANKS;
            return;
        }

        currentChat = ErikCurrentChat::HELLO;
    }

    void onScreen(float fElapsedTime) override
    {
        CoreNPC::onScreen(fElapsedTime);
        evaluateIfCompletedTask();

        switch (currentChat)
        {
        case ErikCurrentChat::HELLO:
            playHelloDialog();
            break;

        case ErikCurrentChat::THANKS:
            playThanksDialog();
            break;

        case ErikCurrentChat::NONE:
        default:
            break;
        }
    }

    void onChatEnded() override
    {
        if (didClearBees && !didShowSecret)
        {
            game->enableLevelPortal();
            didShowSecret = true;
        }

        currentChat = ErikCurrentChat::NONE;
    }

private:
    void playHelloDialog()
    {
        chat({
            "ERIK: Hello, Anderson got my bees angry",
            "ERIK: Please help me to calm them down",
            "ERIK: I'll tell you a secret if you do it",
            "YOU: Sure, I'll help you",
        });
    }

    void playThanksDialog()
    {
        if (didShowSecret)
        {
            chat({
                "ERIK: Go save the world, hero!",
            });

            return;
        }

        chat({
            "ERIK: Thanks for helping me with the bees",
            "ERIK: The secret is that... I'm a bee too",
            "ERIK: Lol, just kidding, I'm a human",
            "ERIK: The secret is that...",
            "ERIK: I can open portals, like in rick and morty",
            "ERIK: I'll open one for you to the city",
            "ERIK: In the city you might find the hex guardian",
            "ERIK: He may provide you with the infinity gem that\nyou need to bring balance to the world",
        });
    }

private:
    void evaluateIfCompletedTask()
    {

        if (didShowSecret)
            return;

        didClearBees = false;

        auto bees = game->getOnScreenChildrenOfType<BeeEnemy>(false);

        for (auto &bee : bees)
        {
            if (!bee->isHarmless())
                return;
        }

        didClearBees = true;
    }
};
REGISTER_NODE_TYPE(ErikNPC, "erik");

#pragma endregion Erik - NPC

#pragma region Martin - NPC

class MartinNPC : public CoreNPC
{
private:
    bool didSetMiniGame = false;
    bool didPlayDialog = false;
    bool didPlaySecondDialog = false;
    bool didWin = false;

public:
    MartinNPC(const ldtk::Entity &entity, GameNode *game) : CoreNPC(entity, game)
    {
    }

    void onCreated() override
    {
        CoreNPC::onCreated();
        animProvider->AddAnimation("idle", 2.0f, {{}, {1, 0}});
        animProvider->PlayAnimation("idle");
        didSetMiniGame = false;
        didPlayDialog = false;
        didWin = false;
    }

    void onAllCreated() override
    {
        CoreNPC::onAllCreated();
        game->setFlag("showGem", false);
    }

    void onInteracted(PlayerNode *player) override
    {
        if (didWin)
        {
            chat({"MARTIN: Thinking..."});
            return;
        }

        if (!didPlayDialog)
        {
            didPlayDialog = true;
            playHelloDialog();
        }
        else if (!didPlaySecondDialog)
        {
            didPlaySecondDialog = true;
            playRetryDialog();
        }
    }

    void onChatEnded() override
    {
        didPlaySecondDialog = false;

        if (didSetMiniGame)
            return;

        didSetMiniGame = true;
        game->setMiniGame("ShellGame");
    }

    void playHelloDialog()
    {
        chat({
            "MARTIN: Thinking... 1/2 (with eyes closed)",
            "MARTIN: Thinking... 2/2 (with eyes closed)",
            "MARTIN: Hello, I'm Martin, the Hex Guardian",
            "MARTIN: I'm here to protect the infinity gem",
            "MARTIN: I know what you are thinking...",
            "MARTIN: And yes...",
            "MARTIN: My armor is fully made of gold",
            "YOU: I wasn't thinking that",
            "MARTIN: I know, I'm just messing with you",
            "YOU: Plus, it looks like plastic",
            "MARTIN: We are in the ancient times, we don't have plastic yet",
            "MARTIN: Anyway, you can't have the gem",
            "MARTIN: You need to prove yourself first",
            "MARTIN: By playing a shell game... with me",
            "MARTIN: If you guess where the gem is, you can have it",
            "YOU: That sounds fair",
        });
    }

    void playRetryDialog()
    {
        didSetMiniGame = false;
        chat({
            "MARTIN: Fine, I can give you another chance",
        });
    }

    void onMiniGameOver(std::string name, bool didWin) override
    {
        if (name != "ShellGame")
            return;

        this->didWin = didWin;

        if (didWin)
        {
            this->game->setFlag("showGem", true);
            chat({
                "MARTIN: You won the game, here is the gem",
                "MARTIN: Use it wisely",
            });
        }
        else
        {
            chat({
                "MARTIN: You lost the game. I'll keep the gem",
            });
        }
    }
};
REGISTER_NODE_TYPE(MartinNPC, "martin");

#pragma endregion Martin - NPC