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
    float delta = 0.0f;
    std::vector<Particle> particles;
    int particlesCount = 80;
    int emissionRate = 5;
    float particleDelta = 0.0f;
    int particleTimes = 0;
    olc::vf2d direction = {1, 0};

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
            map->DrawTile(displayPosition, tilesetId, *this->framePosition, {32.0f, 32.0f}, scale, true);

        if (isSpraying)
        {

            for (auto &p : particles)
            {
                if (p.lifespan <= 0.0f)
                    continue;

                p.Update(fElapsedTime);

                if (p.lifespan <= 0.0f)
                    continue;

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
    }

    void OnInteract(float fElapsedTime) override
    {
        isSpraying = true;

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
    }

    void OnInteract(float fElapsedTime) override
    {
        if (items[selectedSlot] != nullptr)
        {
            items[selectedSlot]->OnInteract(fElapsedTime);
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
        if (isFollowingPlayer)
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
