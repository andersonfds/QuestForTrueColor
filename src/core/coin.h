using namespace olc::utils::geom2d;

class Coin : public Node
{
private:
    olc::vf2d *position;
    olc::vf2d *rectPos;
    int tilesetId;
    Map *map;
    Player *player;
    rect<float> *collider;

public:
    void OnCreate() override
    {
        map = GetLayer()->GetNode<Map>();
        player = GetLayer()->GetNode<Player>();
        const auto &coinEntity = map->GetEntity(this->GetEntityID());
        auto initialPosition = coinEntity.getPosition();
        tilesetId = map->GetTilesetIDByPath(coinEntity.getTexturePath());
        auto textureRect = coinEntity.getTextureRect();
        rectPos = new olc::vf2d({static_cast<float>(textureRect.x), static_cast<float>(textureRect.y)});
        position = new olc::vf2d({static_cast<float>(initialPosition.x), static_cast<float>(initialPosition.y)});
        olc::vf2d offset = {16.0f, 16.0f};
        *position -= offset;
        collider = new rect<float>(*position, {32.0f, 32.0f});
    }

    void OnProcess(float fElapsedTime) override
    {
        Camera *camera = GetLayer()->GetCamera();
        if (camera->IsOnScreen(*position))
        {
            olc::vf2d pos = *position;
            map->DrawTile(pos, tilesetId, *rectPos, {32.0f, 32.0f}, {1.0f, 1.0f});
        }

        // Draw collider
        if (DEBUG)
        {
            olc::vf2d pos = *position;
            camera->WorldToScreen(pos);
            GetEngine()->DrawRectDecal(pos, {32.0f, 32.0f}, olc::WHITE);
        }
    }

    void OnPhysicsProcess(float fElapsedTime) override
    {
        Camera *camera = GetLayer()->GetCamera();
        if (camera->IsOnScreen(*position))
        {
            if (overlaps(*collider, *player->GetCollider()))
            {
                player->AddMoney(1);
                GetLayer()->RemoveNode(this);
            }
        }
    }
};