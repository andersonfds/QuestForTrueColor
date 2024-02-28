#pragma once

class Item : public Node
{
protected:
    olc::vf2d *initialPosition;
    olc::vf2d *framePosition;
    int tilesetId;

public:
    void OnCreate() override
    {
        Map *map = GetLayer()->GetNode<Map>();
        const auto &itemEntity = map->GetEntity(this->GetEntityID());
        auto initialPosition = itemEntity.getPosition();
        this->tilesetId = map->GetTilesetIDByPath(itemEntity.getTexturePath());
        this->initialPosition = new olc::vf2d({static_cast<float>(initialPosition.x), static_cast<float>(initialPosition.y)});
        auto texture = itemEntity.getTextureRect();
        this->framePosition = new olc::vf2d({static_cast<float>(texture.x), static_cast<float>(texture.y)});
        OnEntityDefined(itemEntity);
    }

    virtual void OnEntityDefined(const ldtk::Entity &entity) = 0;

    virtual void OnScreen(float fElapsedTime) = 0;

    void OnProcess(float fElapsedTime) override
    {
        Camera *camera = GetCamera();
        if (!camera->IsOnScreen(*initialPosition))
        {
            return;
        }

        OnScreen(fElapsedTime);
    }

    virtual void OnActivate(olc::vf2d *playerPosition)
    {
    }

    virtual void OnDeactivate()
    {
    }
};

class BugSpray : public Item
{
public:
    void OnEntityDefined(const ldtk::Entity &entity) override
    {
    }

    void OnScreen(float fElapsedTime) override
    {
        Map *map = GetLayer()->GetNode<Map>();
        map->DrawTile(*this->initialPosition, tilesetId, *this->framePosition, {32.0f, 32.0f}, {1.0f, 1.0f});
    }
};

class Purse : public Item
{
public:
    void OnEntityDefined(const ldtk::Entity &entity) override
    {
    }

    void OnScreen(float fElapsedTime) override
    {
        Map *map = GetLayer()->GetNode<Map>();
        map->DrawTile(*this->initialPosition, tilesetId, *this->framePosition, {32.0f, 32.0f}, {1.0f, 1.0f});
    }
};