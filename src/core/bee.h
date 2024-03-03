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
    int direction = 1;
    float timeOffscreen = 0.0f;
    bool didHitPlayer = false;

public:
    void OnCreate() override
    {
        int initialFrame = rand() % 2;
        flying = new AnimationController(this, 0.4f, initialFrame, {0, 1});

        Map *map = GetLayer()->GetNode<Map>();
        const auto &beeEntity = map->GetEntity(this->GetEntityID());
        auto initialPosition = beeEntity.getWorldPosition();

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
        auto travelToX = (this->travelTo != nullptr ? this->travelTo->x : 0) * 32;
        this->direction = initialPosition.x < travelToX ? 1 : -1;
    }

    void OnProcess(float fElapsedTime) override
    {
        Camera *camera = GetLayer()->GetCamera();

        if (!camera->IsOnScreen(*position))
        {
            if (timeOffscreen > 30.0f)
            {
                return;
            }

            timeOffscreen += fElapsedTime;
        }
        else
        {
            timeOffscreen = 0.0f;
        }

        if (travelTo != nullptr)
        {
            delta += fElapsedTime;
            float travelToX = travelTo->x * 32.0f * 0.5f;
            float travelToY = travelTo->y * 32.0f;
            float destinationX = initialPosition->x + (travelToX * direction);
            bool reachedDestinationX = direction == 1 ? position->x >= destinationX : position->x <= destinationX;

            if (reachedDestinationX)
            {
                direction *= -1;
            }

            if (delta > speed)
            {
                delta = 0;
            }

            position->x += direction * speed * fElapsedTime;
            position->y = initialPosition->y + travelToY + sin(delta * 2) * 10;
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

        // Draw collider
        if (DEBUG)
        {
            olc::vf2d pos = *position;
            camera->WorldToScreen(pos);
            GetEngine()->DrawRectDecal(pos, {32.0f, 32.0f}, olc::WHITE);
        }
    }
};