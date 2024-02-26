#pragma once

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
    int direction = 1;

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

        if (!travelToField.is_null())
        {
            ldtk::IntPoint travelTo = travelToField.value();
            this->travelTo = new olc::vf2d({static_cast<float>(travelTo.x), static_cast<float>(travelTo.y)});
        }

        position = new olc::vf2d({static_cast<float>(initialPosition.x), static_cast<float>(initialPosition.y)});
        this->initialPosition = new olc::vf2d(*position);
    }

    void OnProcess(float fElapsedTime) override
    {
        Camera *camera = GetLayer()->GetCamera();

        if (travelTo != nullptr)
        {
            delta += fElapsedTime;
            position->x += speed * fElapsedTime * direction;

            if (position->x > initialPosition->x + travelTo->x * 32)
            {
                delta = 0.0f;
                direction = -1;
            }
            else if (position->x < initialPosition->x)
            {
                delta = 0.0f;
                direction = 1;
            }
            position->y = initialPosition->y + travelTo->y * 32 + sin(delta * 2) * 10;
        }

        if (!camera->IsOnScreen(*position))
        {
            return;
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

        map->DrawTile(pos, flying->tilesetId, frame, {32.0f, 32.0f}, {1.0f * direction, 1.0f});

        // Draw collider
        if (DEBUG)
        {
            olc::vf2d pos = *position;
            camera->WorldToScreen(pos);
            GetEngine()->DrawRectDecal(pos, {32.0f, 32.0f}, olc::WHITE);
        }
    }
};