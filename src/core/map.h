#pragma once

#include <fstream>

using namespace olc::utils::geom2d;

Node *CreateEntity(const ldtk::Entity &entity);

class Map : public Node
{

private:
    ldtk::Project ldtk_project;
    std::string levelName;
    olc::Pixel bgColor;
    std::map<int, std::unique_ptr<olc::Decal>> tilesets;
    std::vector<rect<float>> colliders;
    float distance = 0.0;
    std::vector<Node *> entities;
    bool enableUI = false;
    olc::vf2d *uiCoords;
    olc::Decal *background;
    std::vector<std::string> dialogues;
    float dialogTime = 0.0f;
    bool persist = false;

    const ldtk::Level &getLevel()
    {
        const auto &world = ldtk_project.getWorld();
        return world.getLevel(levelName);
    }

    bool IsOffScreen(olc::vf2d pos)
    {
        Camera *camera = GetCamera();
        return !camera->IsOnScreen(pos);
    }

public:
    void SetActiveLevel(std::string levelName)
    {
        this->levelName = levelName;
        auto &level = getLevel();
        bgColor = olc::Pixel(level.bg_color.r, level.bg_color.g, level.bg_color.b);
        colliders = std::vector<rect<float>>();

        if (level.hasBgImage())
        {
            auto &bgImage = level.getBgImage();
            const auto bgImagePath = bgImage.path.c_str();
            auto fullImagePath = "assets/map_project/" + std::string(bgImagePath);
            auto *sprite = new olc::Sprite(fullImagePath);
            background = new olc::Decal(sprite);
        }

        GetCamera()->size->x = level.size.x;
        GetCamera()->size->y = level.size.y;

        /// Load colliders
        colliders.clear();
        auto &collidersLayer = level.getLayer("colliders");
        for (int x = 0; x < collidersLayer.getGridSize().x; x++)
        {
            for (int y = 0; y < collidersLayer.getGridSize().y; y++)
            {
                auto intGridVal = collidersLayer.getIntGridVal(x, y);
                if (intGridVal.value > 0)
                {
                    auto offset = collidersLayer.getOffset();
                    olc::vf2d offsetPos = {static_cast<float>(offset.x), static_cast<float>(offset.y)};
                    olc::vf2d pos = {static_cast<float>(x * collidersLayer.getCellSize()), static_cast<float>(y * collidersLayer.getCellSize())};
                    pos += offsetPos;
                    olc::vf2d size = {static_cast<float>(collidersLayer.getCellSize()), static_cast<float>(collidersLayer.getCellSize())};
                    colliders.push_back({pos, size});
                }
            }
        }

        // Remove all entities
        for (auto &entity : entities)
        {
            GetLayer()->RemoveNode(entity);
        }
        entities.clear();

        auto &entities = GetAllEntities();

        for (auto &entity : entities)
        {
            Node *node = CreateEntity(entity);

            if (node == nullptr)
                continue;

            node->SetEntityID(entity.iid);
            GetLayer()->AddNode(node);
            node->OnCreate();
        }

        enableUI = level.getField<ldtk::FieldType::Bool>("enable_ui").value_or(false);

        auto &worldEnum = ldtk_project.getWorld().getEnum("world")["base"];
        auto &rect = worldEnum.getIconTextureRect();
        uiCoords = new olc::vf2d(rect.x, rect.y);
    }

    void OnCreate() override
    {
        std::string basePath = "assets/map_project/";
        ldtk_project.loadFromFile(basePath + "QuestForTrueColor.ldtk");

        auto &world = ldtk_project.getWorld();

        auto &tilesets = world.allTilesets();

        for (auto &tileset : tilesets)
        {
            olc::Sprite *sprite = new olc::Sprite(basePath + tileset.path);
            std::unique_ptr<olc::Decal> decal = std::make_unique<olc::Decal>(sprite);
            this->tilesets[tileset.uid] = std::move(decal);
        }

        AddDialog("Welcome to Quest for True Color!");
    }

    std::vector<olc::vf2d> IsColliding(ray<float> *ray, float distance)
    {
        for (auto &box : colliders)
        {
            if (IsOffScreen(box.pos))
                continue;

            line<float> line = {ray->origin, ray->origin + ray->direction * distance};

            std::vector<olc::vf2d> intersection = intersects(line, box);

            if (intersection.size() > 0)
            {
                return intersection;
            }
        }

        return std::vector<olc::vf2d>();
    }

    std::vector<olc::vf2d> IsColliding(rect<float> *rect)
    {
        std::vector<olc::vf2d> intersections;

        for (auto &box : colliders)
        {
            if (IsOffScreen(box.pos))
                continue;

            std::vector<olc::vf2d> intersection = intersects(*rect, box);

            if (intersection.size() > 0)
            {
                intersections.push_back(intersection[0]);
            }
        }

        return intersections;
    }

    void OnProcess(float fElapsedTime) override
    {
        Camera *camera = GetCamera();
        const auto &level = getLevel();

        if (background != nullptr)
        {
            olc::vf2d pos = {0, 0};
            GetEngine()->DrawDecal(pos, background, {1.0f, 1.0f});
        }
        else
        {
            GetEngine()->FillRectDecal({0, 0}, {static_cast<float>(GetEngine()->ScreenWidth()), static_cast<float>(GetEngine()->ScreenHeight())}, bgColor);
        }

        /// Draws the platformer layer
        DrawLayer("platform");

        // Drawing dialog
        if (dialogues.size() > 0)
        {
            if (GetEngine()->GetKey(olc::Key::SPACE).bPressed)
            {
                dialogTime = 0.0f;
                dialogues.erase(dialogues.begin());
            }

            if (!persist)
                dialogTime += fElapsedTime;

            if (dialogTime > 3.0f)
            {
                dialogTime = 0.0f;
                dialogues.erase(dialogues.begin());
            }
            else
            {
                auto textSize = GetEngine()->GetTextSize(dialogues[0]);
                float height = textSize.y + 20;
                GetEngine()->FillRectDecal({10, 10}, {static_cast<float>(GetEngine()->ScreenWidth() - 20.0f), height}, olc::Pixel(0, 0, 0, 200));
                GetEngine()->DrawStringDecal({20, 20}, dialogues[0], olc::WHITE);
            }
        }

        // Draw colliders if debug mode is enabled
        if (DEBUG)
            for (auto &collider : colliders)
            {
                if (IsOffScreen(collider.pos))
                    continue;

                olc::vf2d pos = olc::vf2d(collider.pos);
                camera->WorldToScreen(pos);
                GetEngine()->DrawRectDecal(pos, collider.size, olc::WHITE);
            }
    }

    void DrawLayer(std::string name)
    {
        const auto &level = getLevel();
        auto &layer = level.getLayer(name);
        auto &tileset = layer.getTileset();
        for (auto &tile : layer.allTiles())
        {
            const auto &level = getLevel();

            auto rect = tile.getTextureRect();
            auto worldPos = tile.getWorldPosition();

            olc::vf2d pos = {static_cast<float>(worldPos.x), static_cast<float>(worldPos.y)};
            olc::vf2d tileSize = {static_cast<float>(rect.width), static_cast<float>(rect.height)};

            if (IsOffScreen(pos))
                continue;

            bool flipX = tile.flipX;
            bool flipY = tile.flipY;

            olc::vf2d scale = {flipX ? -1.0f : 1.0f, flipY ? -1.0f : 1.0f};

            if (flipX)
            {
                pos.x += tileSize.x;
            }

            if (flipY)
            {
                pos.y += tileSize.y;
            }

            olc::vf2d tileMap = {static_cast<float>(rect.x), static_cast<float>(rect.y)};
            DrawTile(pos, tileset.uid, tileMap, tileSize, scale);
        }
    }

    void DrawTile(olc::vf2d pos, int tilesetID, olc::vf2d tileMap, olc::vf2d tileSize, olc::vf2d scale = {1.0f, 1.0f}, bool worldToScreen = true)
    {
        Camera *camera = GetCamera();
        olc::Decal *decal = tilesets[tilesetID].get();
        if (worldToScreen)
            camera->WorldToScreen(pos);
        GetEngine()->DrawPartialDecal(pos, decal, tileMap, tileSize, scale);
    }

    const ldtk::Entity &GetEntity(ldtk::IID entityID)
    {
        auto &level = getLevel();
        auto &entitiesLayer = level.getLayer("entities");
        return entitiesLayer.getEntity(entityID);
    }

    const std::vector<ldtk::Entity> &GetAllEntities()
    {
        auto &level = getLevel();
        auto &entitiesLayer = level.getLayer("entities");
        return entitiesLayer.allEntities();
    }

    bool getEnableUI()
    {
        return enableUI;
    }

    void DrawIcon(olc::vf2d pos, olc::vi2d itemPos = {0, 0}, std::string enumValue = "base", float factor = 16.0f, bool worldToScreen = false)
    {
        auto &worldEnum = ldtk_project.getWorld().getEnum("world");
        auto tilesetId = worldEnum.getIconsTileset().uid;
        auto &enumRect = worldEnum[enumValue];
        auto &rect = enumRect.getIconTextureRect();
        auto *uiCoords = new olc::vf2d(rect.x, rect.y);
        *uiCoords += itemPos * factor;
        DrawTile(pos, tilesetId, *uiCoords, {factor, factor}, {1.0f, 1.0f}, worldToScreen);
    }

    int GetTilesetIDByPath(std::string path)
    {
        auto &world = ldtk_project.getWorld();
        auto &tilesets = world.allTilesets();

        for (auto &tileset : tilesets)
        {
            if (tileset.path == path)
            {
                return tileset.uid;
            }
        }

        return -1;
    }

    int GetTilesetIDByName(std::string name)
    {
        auto &world = ldtk_project.getWorld();
        auto &tilesets = world.allTilesets();

        for (auto &tileset : tilesets)
        {
            if (tileset.name == name)
            {
                return tileset.uid;
            }
        }

        return -1;
    }

    void ClearDialogs()
    {
        dialogues.clear();
    }

    void AddDialog(std::string dialogue, bool persist = false)
    {
        if (persist)
        {
            this->persist = true;
        }

        dialogues.push_back(dialogue);
    }

    bool HasDialogs()
    {
        return dialogues.size() > 0;
    }
};