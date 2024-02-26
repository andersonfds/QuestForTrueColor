#pragma once

class Bee : public Node
{
private:
    bool isMad = false;
    AnimationController *flying;
    olc::vf2d *position;

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

        position = new olc::vf2d({static_cast<float>(initialPosition.x), static_cast<float>(initialPosition.y)});
    }

    void OnProcess(float fElapsedTime) override
    {
        Camera *camera = GetLayer()->GetCamera();
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

        map->DrawTile(pos, flying->tilesetId, frame, {32.0f, 32.0f}, {1.0f, 1.0f});

        // Draw collider
        if (DEBUG)
        {
            olc::vf2d pos = *position;
            camera->WorldToScreen(pos);
            GetEngine()->DrawRectDecal(pos, {32.0f, 32.0f}, olc::WHITE);
        }
    }
};