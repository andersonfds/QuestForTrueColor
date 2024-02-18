#pragma once

#include <fstream>

using namespace olc::utils::geom2d;

// struct TileSet
// {
//     std::unique_ptr<olc::Decal> image;
//     int firstgid;
//     int tilecount;
//     int columns;
//     std::string name;
// };

// struct MapLayer
// {
//     std::string name;
//     std::vector<int> data;
//     int width;
//     int height;
// };

// olc::Pixel *GetColor(std::string hex)
// {
//     int r = std::stoi(hex.substr(1, 2), 0, 16);
//     int g = std::stoi(hex.substr(3, 2), 0, 16);
//     int b = std::stoi(hex.substr(5, 2), 0, 16);

//     return new olc::Pixel(r, g, b);
// }

// class Map
// {
// public:
//     Map(std::string path)
//     {
//         std::ifstream f(path);
//         data = json::parse(f);
//         bgColor = GetColor(data["backgroundcolor"]);
//         tilesets = std::vector<TileSet>();
//         layers = std::vector<MapLayer>();

//         std::string basePath = path.substr(0, path.find_last_of("/\\"));

//         for (auto tileset : data["tilesets"])
//         {
//             std::string imagePath = tileset["image"];
//             std::string fullPath = basePath + "/" + imagePath;

//             olc::Sprite *sprite = new olc::Sprite(fullPath);
//             std::unique_ptr<olc::Decal> decal = std::make_unique<olc::Decal>(sprite);

//             int firstgid = tileset["firstgid"];
//             int tilecount = tileset["tilecount"];
//             int columns = tileset["columns"];

//             tilesets.push_back({std::move(decal), firstgid, tilecount, columns, tileset["name"]});
//         }

//         for (auto layer : data["layers"])
//         {
//             std::string name = layer["name"];
//             std::vector<int> data = layer["data"];
//             int height = layer["height"];
//             int width = layer["width"];
//             layers.push_back({name, data, width, height});
//         }
//     }

//     void Render(olc::PixelGameEngine *pge)
//     {
//         pge->FillRectDecal({0, 0}, pge->GetScreenSize(), *bgColor);
//         auto layer = layers.at(0);
//         TileSet *tileset = &tilesets.at(0);
//         olc::vf2d tileSize = {32, 32};

//         for (int i = 0; i < layer.data.size(); i++)
//         {
//             int gindex = layer.data.at(i) - 1;
//             int currCol = i % layer.width;
//             int currRow = i / layer.width;
//             olc::vf2d pos = {(float)currCol, (float)currRow};

//             int tileX = gindex % tileset->columns;
//             int tileY = gindex / tileset->columns;

//             olc::vf2d tilePos = {tileX * tileSize.x, tileY * tileSize.y};

//             pge->DrawPartialDecal(pos * tileSize, tileset->image.get(), tilePos, tileSize);
//         }
//     }

// private:
//     json data;
//     olc::Pixel *bgColor;
//     std::vector<TileSet> tilesets;
//     std::vector<MapLayer> layers;
// };

class Map : public Node
{

private:
    ldtk::Project ldtk_project;
    std::string levelName;
    olc::Pixel bgColor;
    std::map<int, std::unique_ptr<olc::Decal>> tilesets;
    std::vector<rect<float>> colliders;

    const ldtk::Level &getLevel()
    {
        const auto &world = ldtk_project.getWorld();
        return world.getLevel(levelName);
    }

    void setActiveLevel(std::string levelName)
    {
        this->levelName = levelName;
        auto &level = getLevel();
        bgColor = olc::Pixel(level.bg_color.r, level.bg_color.g, level.bg_color.b);
        colliders = std::vector<rect<float>>();

        auto &collidersLayer = level.getLayer("colliders");

        for (int x = 0; x < collidersLayer.getGridSize().x; x++)
        {
            for (int y = 0; y < collidersLayer.getGridSize().y; y++)
            {
                auto intGridVal = collidersLayer.getIntGridVal(x, y);
                if (intGridVal.value > 0)
                {
                    olc::vf2d pos = {static_cast<float>(x * collidersLayer.getCellSize()), static_cast<float>(y * collidersLayer.getCellSize())};
                    olc::vf2d size = {static_cast<float>(collidersLayer.getCellSize()), static_cast<float>(collidersLayer.getCellSize())};
                    colliders.push_back({pos, size});
                }
            }
        }
    }

    bool IsOffScreen(olc::vf2d pos)
    {
        return pos.x < 0 || pos.y < 0 || pos.x > GetEngine()->ScreenWidth() || pos.y > GetEngine()->ScreenHeight();
    }

public:
    void OnCreate() override
    {
        std::string basePath = "assets/map_project/";
        ldtk_project.loadFromFile(basePath + "QuestForTrueColor.ldtk");
        setActiveLevel("level_1");

        auto &world = ldtk_project.getWorld();

        auto &tilesets = world.allTilesets();

        for (auto &tileset : tilesets)
        {
            olc::Sprite *sprite = new olc::Sprite(basePath + tileset.path);
            std::unique_ptr<olc::Decal> decal = std::make_unique<olc::Decal>(sprite);
            this->tilesets[tileset.uid] = std::move(decal);
        }
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

    void OnProcess(float fElapsedTime) override
    {
        const auto &level = getLevel();

        GetEngine()->FillRectDecal({0, 0}, {static_cast<float>(GetEngine()->ScreenWidth()), static_cast<float>(GetEngine()->ScreenHeight())}, bgColor);

        for (auto &layer : level.allLayers())
        {
            auto &tileset = layer.getTileset();
            for (auto &tile : layer.allTiles())
            {
                auto rect = tile.getTextureRect();
                auto worldPos = tile.getWorldPosition();

                olc::vf2d pos = {static_cast<float>(worldPos.x), static_cast<float>(worldPos.y)};
                olc::vf2d tileSize = {static_cast<float>(rect.width), static_cast<float>(rect.height)};

                if (pos.x < 0 || pos.y < 0 || pos.x > GetEngine()->ScreenWidth() || pos.y > GetEngine()->ScreenHeight())
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

                auto tilesetId = tileset.uid;
                olc::Decal *decal = tilesets[tilesetId].get();
                GetEngine()->DrawPartialDecal(pos, decal, {static_cast<float>(rect.x), static_cast<float>(rect.y)}, tileSize, scale);
            }
        }

        if (DEBUG)
            for (auto &collider : colliders)
            {
                // dont draw offscreen colliders
                if (collider.pos.x < 0 || collider.pos.y < 0 || collider.pos.x > GetEngine()->ScreenWidth() || collider.pos.y > GetEngine()->ScreenHeight())
                    continue;

                GetEngine()->DrawRectDecal(collider.pos, collider.size, olc::WHITE);
            }
    }
};