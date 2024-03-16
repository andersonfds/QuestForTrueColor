#pragma once

using namespace olc::utils::geom2d;

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
};