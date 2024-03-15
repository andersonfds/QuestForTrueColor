using namespace olc::utils::geom2d;

class Coin : public Item
{
private:
    olc::vf2d *position;
    olc::vf2d *rectPos;
    Map *map;
    Player *player;
    rect<float> *collider;
    AnimationController *animationController;

public:
    void OnPhysicsProcess(float fElapsedTime) override
    {
        Camera *camera = GetLayer()->GetCamera();
        player = GetLayer()->GetNode<Player>();

        if (camera->IsOnScreen(*position))
        {
            if (player == nullptr)
                return;
            if (overlaps(*collider, *player->GetCollider()))
            {
                player->AddMoney(1);
                GetLayer()->RemoveNode(this);
            }
        }
    }

    void OnEntityDefined(const ldtk::Entity &entity) override
    {
        map = GetLayer()->GetNode<Map>();
        player = GetLayer()->GetNode<Player>();
        const auto &coinEntity = map->GetEntity(this->GetEntityID());
        auto initialPosition = coinEntity.getPosition();
        position = new olc::vf2d({static_cast<float>(initialPosition.x), static_cast<float>(initialPosition.y)});
        collider = new rect<float>(*position, {32.0f, 32.0f});
        int initialFrame = rand() % 5;
        animationController = new AnimationController(this, 0.1f, initialFrame, {0, 1, 2, 3, 4});
    }

    void OnScreen(float fElapsedTime) override
    {
        Camera *camera = GetLayer()->GetCamera();
        auto frame = animationController->GetFrame(fElapsedTime);
        olc::vf2d pos = *position;
        map->DrawTile(pos, animationController->tilesetId, frame, {32.0f, 32.0f}, {1.0f, 1.0f});

        // Draw collider
        if (DEBUG)
        {
            olc::vf2d pos = *position;
            camera->WorldToScreen(pos);
            GetEngine()->DrawRectDecal(pos, {32.0f, 32.0f}, olc::WHITE);
        }
    }
};