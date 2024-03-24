#pragma once

using namespace olc::utils::geom2d;

olc::vf2d RotateVector(const olc::vf2d &vec, float angle)
{
    float cosA = cos(angle);
    float sinA = sin(angle);
    return {vec.x * cosA - vec.y * sinA, vec.x * sinA + vec.y * cosA};
}

class Particle
{
public:
    olc::vf2d position;
    olc::vf2d velocity;
    float lifespan;

    Particle(olc::vf2d startPos, olc::vf2d startVel, float life) : position(startPos), velocity(startVel), lifespan(life) {}

    // Update the particle's position and lifespan
    void Update(float fElapsedTime)
    {
        position += velocity * fElapsedTime;
        lifespan -= fElapsedTime;
    }
};

class BugSpray : public Item
{
private:
    bool isFollowingPlayer;
    bool isSpraying;
    bool didSprayOnce;
    float delta = 0.0f;
    std::vector<Particle> particles;
    int particlesCount = 80;
    int emissionRate = 5;
    float particleDelta = 0.0f;
    int particleTimes = 0;
    olc::vf2d direction = {1, 0};
    Sound *sfx;

public:
    void OnCreate() override
    {
        Item::OnCreate();
        isFollowingPlayer = false;
        isSpraying = false;
        particles.clear();
        for (int i = 0; i < particlesCount; ++i)
            particles.emplace_back(olc::vf2d{0, 0}, olc::vf2d{0, 0}, 0.0f);
        isSpraying = false;
        delta = 0.0f;
        particleTimes = 0;
        didSprayOnce = false;
        sfx = new Sound("assets/sfx/spray.wav");
    }

    void OnScreen(float fElapsedTime) override
    {
        Map *map = GetLayer()->GetNode<Map>();
        Player *player = GetLayer()->GetNode<Player>();
        olc::vf2d displayPosition = *this->position;
        olc::vf2d scale = {1.0f, 1.0f};
        bool overlapsPlayer = false;
        int playerDirection = player->IsFacingRight() ? 1 : -1;
        direction.x = playerDirection;

        if (!isFollowingPlayer)
        {
            auto playerCollider = player->GetCollider();
            overlapsPlayer = overlaps(*playerCollider, GetCollider());
            if (overlapsPlayer)
            {
                player->CanActivate(this);
            }
            else
            {
                player->ExitItem(this);
            }

            // snap position to grid 32x32
            position->x = ((int)displayPosition.x + 16) / 32 * 32;
            position->y = ((int)displayPosition.y + 16) / 32 * 32;
        }

        if (!isFollowingPlayer)
        {
            delta += fElapsedTime;

            if (delta > 1.0f)
            {
                delta = 0.0f;
            }

            displayPosition.y += sin(delta * 3.14159f) * 5.0f;
        }
        else
        {
            *position = *player->GetPosition();
            scale.x = playerDirection;
            if (!player->IsFacingRight())
            {
                displayPosition.x += 32.0f;
            }
        }

        if (render)
        {
            map->DrawTile(displayPosition, tilesetId, *this->framePosition, {32.0f, 32.0f}, scale, true);
        }

        if (isSpraying)
        {
            bool livingParticles = false;

            for (auto &p : particles)
            {
                if (p.lifespan <= 0.0f)
                    continue;

                p.Update(fElapsedTime);

                if (p.lifespan <= 0.0f)
                    continue;

                livingParticles = true;

                int draw = p.lifespan > 1.0f ? 0 : 1;

                map->DrawIcon(p.position, {0, draw}, "spray", 32, true);
            }
        }

        if (!isFollowingPlayer && overlapsPlayer)
        {
            auto text = "Bug Spray";
            auto textSize = GetEngine()->GetTextSize(text) * 0.5f;
            Camera *camera = GetLayer()->GetCamera();
            camera->WorldToScreen(displayPosition);
            displayPosition -= textSize;
            displayPosition.x += 16.0f;
            GetEngine()->DrawStringDecal(displayPosition, text, olc::WHITE);
        }
        else if (isFollowingPlayer && !didSprayOnce && render)
        {
            auto text = "Press Z to spray";
            auto textSize = GetEngine()->GetTextSize(text) * 0.5f;
            Camera *camera = GetLayer()->GetCamera();
            camera->WorldToScreen(displayPosition);
            displayPosition -= textSize;
            displayPosition.x += 16.0f;
            displayPosition.y -= 32.0f;

            if (!player->IsFacingRight())
            {
                displayPosition.x -= 32.0f;
            }

            GetEngine()->DrawStringDecal(displayPosition, text, olc::WHITE);
        }
    }

    void OnInteract(float fElapsedTime) override
    {
        isSpraying = true;
        didSprayOnce = true;

        if (isFollowingPlayer && render)
        {
            sfx->SetPlayed(false);
            sfx->Play();
        }

        if (particleDelta > 0.2f || particleDelta == 0.0f)
        {
            particleDelta = 0.0f;
            SetupParticles();
        }

        particleDelta += fElapsedTime;
    }

    void OnActivate() override
    {
        isFollowingPlayer = true;
    }

    void OnDeactivate() override
    {
        isFollowingPlayer = false;
    }

    void SetupParticles()
    {
        int emitted = 0;
        for (auto &p : particles)
        {
            if (p.lifespan > 0.0f)
                continue;

            if (++emitted > emissionRate)
                break;

            float angle = ((rand() % 45) - 22.5f) * 3.14159f / 180.0f;
            olc::vf2d vel = RotateVector(direction, angle) * (float)(rand() % 100 + 50);
            p.position = *position;
            p.lifespan = 2.0f;
            p.velocity = vel;
        }
    }

    bool IsColliding(rect<float> &collideRect)
    {
        for (auto &p : particles)
        {
            if (p.lifespan <= 0.0f)
                continue;

            rect<float> particleRect = rect<float>({p.position, {32.0f, 32.0f}});

            if (overlaps(particleRect, collideRect))
                return true;
        }

        return false;
    }

    bool IsSpraying()
    {
        return isSpraying;
    }
};

class Purse : public Item
{
private:
    bool isFollowingPlayer;
    olc::vf2d direction;
    float delta = 0.0f;
    int slots;
    int selectedSlot = -1;
    std::vector<Item *> items;
    Item *candidateItem;

public:
    void OnEntityDefined(const ldtk::Entity &entity) override
    {
        isFollowingPlayer = false;
        slots = entity.getField<ldtk::FieldType::Int>("slots").value_or(1);
        items.clear();
        items.resize(slots);
        selectedSlot = -1;
        candidateItem = nullptr;
    }

    void OnScreen(float fElapsedTime) override
    {
        Map *map = GetLayer()->GetNode<Map>();
        Player *player = GetLayer()->GetNode<Player>();
        olc::vf2d displayPosition = *this->position;
        olc::vf2d scale = {1.0f, 1.0f};
        bool overlapsPlayer = false;

        if (!isFollowingPlayer)
        {
            auto playerCollider = player->GetCollider();
            overlapsPlayer = overlaps(*playerCollider, GetCollider());
            if (overlapsPlayer)
            {
                player->CanActivate(this);
            }
            else
            {
                player->ExitItem(this);
            }

            // snap position to grid 32x32
            position->x = ((int)displayPosition.x + 16) / 32 * 32;
            position->y = ((int)displayPosition.y + 16) / 32 * 32;
        }

        if (!isFollowingPlayer)
        {
            delta += fElapsedTime;

            if (delta > 1.0f)
            {
                delta = 0.0f;
            }

            displayPosition.y += sin(delta * 3.14159f) * 5.0f;
        }
        else
        {
            *position = *player->GetPosition();
            int playerDirection = player->IsFacingRight() ? 1 : -1;
            direction.x = playerDirection;
            scale.x = playerDirection;
            if (!player->IsFacingRight())
            {
                displayPosition.x += 32.0f;
            }
        }

        if (!isFollowingPlayer)
            map->DrawTile(displayPosition, tilesetId, *this->framePosition, {32.0f, 32.0f}, scale, true);

        if (!isFollowingPlayer && overlapsPlayer)
        {
            auto text = "Tiny Purse";
            auto textSize = GetEngine()->GetTextSize(text) * 0.5f;
            Camera *camera = GetLayer()->GetCamera();
            camera->WorldToScreen(displayPosition);
            displayPosition -= textSize;
            displayPosition.x += 16.0f;
            GetEngine()->DrawStringDecal(displayPosition, text, olc::WHITE);
        }

        if (isFollowingPlayer && map->getEnableUI())
        {
            olc::vf2d slotPosition = olc::vf2d{0, 0};

            float spacing = 5.0f;
            auto width = GetEngine()->ScreenWidth();
            auto height = GetEngine()->ScreenHeight();

            slotPosition.x = width / 2.0f - (slots * 32) / 2.0f - (slots - 1) * spacing;
            slotPosition.y = height - 32.0f;
            slotPosition.y -= 10.0f; // padding

            for (int i = 0; i < slots; i++)
            {
                auto pos = slotPosition + olc::vf2d{32.0f * i, 0};

                if (i > 0)
                {
                    pos.x += spacing * i;
                }

                bool isSlotEmpty = items[i] == nullptr;

                int slotDraw = i == selectedSlot ? 1 : 0;

                map->DrawIcon(pos, {slotDraw, 0}, "slot", 32, false);

                if (isSlotEmpty)
                {
                    auto text = std::to_string(i + 1);
                    auto textSize = GetEngine()->GetTextSize(text) * 0.5f;
                    pos += olc::vf2d{16.0f, 16.0f};
                    olc::Pixel color = olc::WHITE;

                    if (i != selectedSlot)
                    {
                        color.a = 100;
                    }

                    GetEngine()->DrawStringDecal(pos - textSize, text, color);
                }
                else
                {
                    items[i]->render = i == selectedSlot;
                    map->DrawTile(pos, items[i]->tilesetId, *items[i]->framePosition, {32.0f, 32.0f}, {1.0f, 1.0f}, false);
                }
            }
        }
    }

    void OnExitInteract() override
    {
        candidateItem = nullptr;
    }

    void CanEnableOther(Item *item) override
    {
        candidateItem = item;
        if (items.size() > 0 && selectedSlot >= 0)
        {
            if (items[selectedSlot] == nullptr)
            {
                items[selectedSlot] = candidateItem;
                candidateItem->OnActivate();
            }
        }
    }

    void OnInteract(float fElapsedTime) override
    {
        if (items.size() > 0)
        {
            if (items[selectedSlot] != nullptr)
            {
                items[selectedSlot]->OnInteract(fElapsedTime);
            }
        }
    }

    void OnActivate() override
    {
        isFollowingPlayer = true;
        SetRenderItem(true);
    }

    void OnDeactivate() override
    {
        isFollowingPlayer = false;
        SetRenderItem(false);
    }

    void OnProcess(float fElapsedTime) override
    {
        bool isAlive = !GetLayer()->GetNode<Player>()->IsDead();
        if (isFollowingPlayer && isAlive)
            for (int i = 0; i < slots; i++)
            {
                if (GetEngine()->GetKey((olc::Key)(olc::Key::K1 + i)).bPressed)
                {
                    if (items[i] != nullptr && i == selectedSlot)
                    {
                        items[i]->OnDeactivate();
                        items[i] = nullptr;
                        break;
                    }

                    if (candidateItem != nullptr)
                    {
                        if (items[i] != nullptr)
                        {
                            items[i]->OnDeactivate();
                        }

                        items[i] = candidateItem;
                        candidateItem->OnActivate();
                        candidateItem = nullptr;
                    }

                    selectedSlot = i;
                }
            }

        Item::OnProcess(fElapsedTime);
    }

    void SetRenderItem(bool render)
    {
        if (selectedSlot != -1)
        {
            auto item = items[selectedSlot];
            if (item != nullptr)
            {
                item->render = render;
            }
        }
    }
};

class Checkpoint : public Item
{
private:
    bool isActivated;

public:
    void OnEntityDefined(const ldtk::Entity &entity) override
    {
        Item::OnEntityDefined(entity);
        isActivated = false;
    }

    void OnScreen(float fElapsedTime) override
    {
        Map *map = GetLayer()->GetNode<Map>();
        olc::vf2d displayPosition = *this->position;
        olc::vf2d scale = {1.0f, 1.0f};
        displayPosition.y += 5.0f;
        olc::vi2d tile = {0, 0};

        if (isActivated)
        {
            tile.x = 1;
        }

        if (render)
            map->DrawIcon(displayPosition, tile, "checkpoint", 32, true);
    }

    void OnPhysicsProcess(float fElapsedTime) override
    {
        // if overlaps with user activates it
        if (overlaps(*GetLayer()->GetNode<Player>()->GetCollider(), GetCollider()))
        {
            GetLayer()->GetNode<Player>()->SetCheckpoint(*this->position);
            isActivated = true;
        }
    }
};

class Flower : public Item
{
private:
    bool isFollowingPlayer;
    float delta = 0.0f;
    olc::vf2d direction;

public:
    void OnCreate() override
    {
        Item::OnCreate();
        isFollowingPlayer = false;
        delta = 0.0f;
    }

    void OnScreen(float fElapsedTime) override
    {
        Map *map = GetLayer()->GetNode<Map>();
        Player *player = GetLayer()->GetNode<Player>();
        olc::vf2d displayPosition = *this->position;
        olc::vf2d scale = {1.0f, 1.0f};
        bool overlapsPlayer = false;
        int playerDirection = player->IsFacingRight() ? 1 : -1;
        direction.x = playerDirection;

        if (!isFollowingPlayer)
        {
            auto playerCollider = player->GetCollider();
            overlapsPlayer = overlaps(*playerCollider, GetCollider());
            if (overlapsPlayer)
            {
                player->CanActivate(this);
            }
            else
            {
                player->ExitItem(this);
            }

            position->x = ((int)displayPosition.x + 16) / 32 * 32;
            position->y = ((int)displayPosition.y + 16) / 32 * 32;
        }

        if (!isFollowingPlayer)
        {
            delta += fElapsedTime;

            if (delta > 1.0f)
            {
                delta = 0.0f;
            }

            displayPosition.y += sin(delta * 3.14159f) * 5.0f;
        }
        else
        {
            *position = *player->GetPosition();
            scale.x = playerDirection;
            if (!player->IsFacingRight())
            {
                displayPosition.x += 32.0f;
            }
        }

        if (render)
            map->DrawTile(displayPosition, tilesetId, *this->framePosition, {32.0f, 32.0f}, scale, true);

        if (!isFollowingPlayer && overlapsPlayer)
        {
            auto text = "Unknown Flower";
            auto textSize = GetEngine()->GetTextSize(text) * 0.5f;
            Camera *camera = GetLayer()->GetCamera();
            camera->WorldToScreen(displayPosition);
            displayPosition -= textSize;
            displayPosition.x += 16.0f;
            GetEngine()->DrawStringDecal(displayPosition, text, olc::WHITE);
        }
    }

    void OnActivate() override
    {
        isFollowingPlayer = true;
        Player *player = GetLayer()->GetNode<Player>();
        player->SetHasFlower(true);
    }

    void OnDeactivate() override
    {
        isFollowingPlayer = false;
        Player *player = GetLayer()->GetNode<Player>();
        player->SetHasFlower(false);
    }
};

class TeleportPoint : public Item
{
private:
    float delta = 0.0f;
    olc::vf2d direction;
    AnimationController *portal;
    bool enabled = false;
    std::string targetLevel;

public:
    void OnCreate() override
    {
        Item::OnCreate();
        delta = 0.0f;
        portal = new AnimationController(this, 0.4f, 0, {0, 1, 0, 1});
    }

    void Enable()
    {
        enabled = true;
    }

    void OnEntityDefined(const ldtk::Entity &entity) override
    {
        Item::OnEntityDefined(entity);
        enabled = entity.getField<ldtk::FieldType::Bool>("enabled").value_or(false);
        targetLevel = entity.getField<ldtk::FieldType::String>("level").value_or("");
    }

    void OnScreen(float fElapsedTime) override
    {

        Map *map = GetLayer()->GetNode<Map>();
        Camera *camera = GetLayer()->GetCamera();
        Player *player = GetLayer()->GetNode<Player>();
        olc::vf2d displayPosition = *this->position;
        olc::vf2d scale = {1.0f, 1.0f};
        bool overlapsPlayer = false;
        int playerDirection = player->IsFacingRight() ? 1 : -1;
        direction.x = playerDirection;

        auto playerCollider = player->GetCollider();
        overlapsPlayer = overlaps(*playerCollider, GetCollider());

        auto frame = portal->GetFrame(fElapsedTime);

        if (enabled)
        {
            if (overlapsPlayer)
            {
                map->SetActiveLevel(targetLevel);
                GetLayer()->RemoveNode(this);
            }
            map->DrawTile(displayPosition, portal->tilesetId, frame, {32.0f, 32.0f}, scale, true);
        }

        if (DEBUG)
            GetEngine()->DrawRectDecal(displayPosition, {32.0f, 32.0f}, olc::WHITE);
    }
};

class Bee : public Node
{
private:
    bool isMad = false;
    AnimationController *flying;
    olc::vf2d *position;
    olc::vf2d *travelTo;
    olc::vf2d *initialPosition;
    float speed = 100.0f;
    float delta = 0.0f;
    int direction = 0;
    float timeOffscreen = 0.0f;
    bool didHitPlayer = false;

public:
    void OnCreate() override
    {
        int initialFrame = rand() % 2;
        flying = new AnimationController(this, 0.4f, initialFrame, {0, 1});

        Map *map = GetLayer()->GetNode<Map>();
        const auto &beeEntity = map->GetEntity(this->GetEntityID());
        auto initialPosition = beeEntity.getPosition();

        const auto &isMadField = beeEntity.getField<ldtk::FieldType::Bool>("is_mad");

        if (!isMadField.is_null())
        {
            this->isMad = isMadField.value();
        }

        const auto &travelToField = beeEntity.getField<ldtk::FieldType::Point>("travel");

        position = new olc::vf2d({static_cast<float>(initialPosition.x), static_cast<float>(initialPosition.y)});
        this->initialPosition = new olc::vf2d(*position);

        if (!travelToField.is_null())
        {
            ldtk::IntPoint travelTo = travelToField.value();
            this->travelTo = new olc::vf2d({static_cast<float>(travelTo.x), static_cast<float>(travelTo.y)});
            *this->travelTo *= 32;
        }

        direction = rand() % 2 == 0 ? -1 : 1;
    }

    void OnProcess(float fElapsedTime) override
    {
        Camera *camera = GetLayer()->GetCamera();

        if (travelTo != nullptr)
        {
            float start = initialPosition->x;
            float end = travelTo->x;
            delta += fElapsedTime * 2;

            if (delta > 2 * M_PI)
            {
                delta = 0;
            }

            position->x += speed * fElapsedTime * direction;

            if (position->x < start)
            {
                position->x = start;
                direction = 1;
            }
            else if (position->x > end)
            {
                position->x = end;
                direction = -1;
            }

            position->y = initialPosition->y + sin(delta) * 10;
        }

        Map *map = GetLayer()->GetNode<Map>();
        auto frame = flying->GetFrame(fElapsedTime);
        olc::vf2d pos = *position;

        if (isMad)
        {
            frame.x += 32 * 2;
        }

        if (direction == -1)
        {
            pos.x += 32;
        }

        if (isMad)
        {
            Player *player = GetLayer()->GetNode<Player>();
            rect<float> beeCollider = {pos, {32.0f, 32.0f}};
            if (overlaps(*player->GetCollider(), beeCollider))
            {
                if (!didHitPlayer)
                {
                    player->TakeDamage(1);
                    didHitPlayer = true;
                }
            }
            else
            {
                didHitPlayer = false;
            }

            BugSpray *bugSpray = GetLayer()->GetNode<BugSpray>();
            if (bugSpray != nullptr && bugSpray->IsSpraying())
            {
                auto beeCollider = rect<float>{*position, {32.0f, 32.0f}};
                if (bugSpray->IsColliding(beeCollider))
                {
                    isMad = false;
                }
            }
        }

        if (camera->IsOnScreen(pos))
        {
            map->DrawTile(pos, flying->tilesetId, frame, {32.0f, 32.0f}, {1.0f * direction, 1.0f});

            // Draw collider
            if (DEBUG)
            {
                olc::vf2d pos = *position;
                camera->WorldToScreen(pos);
                GetEngine()->DrawRectDecal(pos, {32.0f, 32.0f}, olc::WHITE);

                auto initialPosScreen = *initialPosition;
                camera->WorldToScreen(initialPosScreen);

                if (travelTo != nullptr)
                {
                    auto travelToScreen = *travelTo;
                    camera->WorldToScreen(travelToScreen);

                    GetEngine()->FillRectDecal(initialPosScreen, {5.0f, 5.0f}, olc::RED);
                    GetEngine()->FillRectDecal(travelToScreen, {5.0f, 5.0f}, olc::RED);
                }
            }
        }
    }

    bool GetIsMad()
    {
        return isMad;
    }
};

class Erik : public Item
{
private:
    AnimationController *idleErik;
    bool didFinishQuest = false;
    bool didInteract = false;
    bool didFinishTalking = false;
    bool didCongratulate = false;
    bool didOpenPortal = false;

public:
    void OnEntityDefined(const ldtk::Entity &entity) override
    {
        Item::OnEntityDefined(entity);
        idleErik = new AnimationController(this, 0.4f, 0, {0, 1, 0, 1});
    }

    void OnScreen(float fElapsedTime) override
    {
        Player *player = GetLayer()->GetNode<Player>();
        Map *map = GetLayer()->GetNode<Map>();
        bool overlapsPlayer = overlaps(*player->GetCollider(), GetCollider());
        didFinishTalking = !map->HasDialogs();

        if (overlapsPlayer && !didInteract && !didFinishQuest)
        {
            map->AddDialog("ERIK: Oh no! All my bees turned red! \nI need to find a way to turn them back to normal.", true);
            map->AddDialog("ERIK: D'you think you can help me?", true);
            map->AddDialog("ERIK: I'll give you a reward if you do!", true);
            map->AddDialog("ERIK: It ain't much, but it's honest work.", true);
            map->AddDialog("YOU : Sure thing!", true);
            map->AddDialog("*PLS HELP ERIK TURN HIS BEES BACK TO NORMAL*", true);
            didInteract = true;
        }

        if (didInteract)
        {
            player->DisableControls();

            if (didFinishTalking)
            {
                player->EnableControls();
                EvaluateWhetherAllBeesAreBackToNormal();
            }
        }

        if (didFinishQuest && overlapsPlayer && !didCongratulate)
        {
            didCongratulate = true;
            player->AddMoney(2);
            map->AddDialog("ERIK: Thanks for helping me out!\nHere's a little something for your trouble.", true);
            map->AddDialog("ERIK: This anderson guy is terrible", true);
            map->AddDialog("ERIK: Since you helped me, maybe you should stop him", true);
            map->AddDialog("ERIK: I think you are strong enough", true);
            map->AddDialog("ERIK: I only have weak little arms", true);
            map->AddDialog("ERIK: FYI, he's headed east, he said that\nlike, 10 seconds ago before leaving this mess.", true);
            map->AddDialog("ERIK: Also I can open portals, I've learnt that from Rick and Morty", true);
            map->AddDialog("ERIK: I'll opena portal for you. It's gonna take you to the city center. Maybe there are more ppl there that need your help", true);
        }

        if (didCongratulate && didFinishTalking && !didOpenPortal)
        {
            didOpenPortal = true;
            TeleportPoint *portal = GetLayer()->GetNode<TeleportPoint>();
            portal->Enable();
        }

        auto position = *this->position;
        position.y += 5.0f;
        olc::vf2d scale = {1.0f, 1.0f};
        auto frame = idleErik->GetFrame(fElapsedTime);
        map->DrawTile(position, tilesetId, frame, {32.0f, 32.0f}, scale, true);
    }

private:
    void EvaluateWhetherAllBeesAreBackToNormal()
    {
        if (didFinishQuest)
            return;

        auto bees = GetLayer()->GetNodes<Bee>();

        for (auto bee : bees)
        {
            if (bee->GetIsMad())
            {
                return;
            }
        }

        didFinishQuest = true;
    }
};