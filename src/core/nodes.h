#pragma once

class GameNode;

CoreNode *CreateNode(GameNode *node, const ldtk::Entity &entity);

struct Dialog
{
    std::string message;
    float duration = 1.0f;
    bool fullscreen = false;
    bool persistent = false;
    uint8_t id = 0;
};

class GameNode : public CoreNode
{

private:
    std::string selectedLevel = "level_1";
    ldtk::Project project;
    GameImageAssetProvider *backgroundProvider;
    std::vector<olc::utils::geom2d::rect<float> *> colliders;
    std::vector<olc::utils::geom2d::rect<float> *> onScreenColliders;
    std::vector<Dialog> dialogs;
    std::map<std::string, bool> flags;

public:
    bool isGameOver = false;
    Camera *camera;
    GameImageAssetProvider *spritesProvider;
    CoreNode *uiNode;

    GameNode(CoreNode *uiNode) : CoreNode("Game", nullptr)
    {
        onScreenColliders.reserve(100);
        dialogs.reserve(10);
        this->uiNode = uiNode;
    }

    ~GameNode() override
    {
        delete camera;
        delete spritesProvider;
        delete backgroundProvider;
        for (auto &collider : colliders)
        {
            delete collider;
        }
    }

    const ldtk::Enum &getGameEnum(const std::string &name)
    {
        return project.getWorld().getEnum(name);
    }

    void onCreated() override
    {
        CoreNode::onCreated();

        camera = new Camera();
        clearDialogs();
        addDialog({"Developed by Anderson, with love and coffee <3", 3.0f, true, false});

        colliders.clear();
        project.loadFromFile("assets/map_project/QuestForTrueColor.ldtk");
        spritesProvider = new GameImageAssetProvider("assets/sprite_project/Sprites.png");
        auto &world = project.getWorld();
        auto &level = world.getLevel(selectedLevel);
        auto &bgImage = level.getBgImage();
        const auto bgImagePath = bgImage.path.c_str();
        auto fullImagePath = "assets/map_project/" + std::string(bgImagePath);
        backgroundProvider = new GameImageAssetProvider(fullImagePath);

        camera->size->x = level.size.x;
        camera->size->y = level.size.y;
        isGameOver = false;

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
                    colliders.push_back(new olc::utils::geom2d::rect<float>(pos, size));
                }
            }
        }

        auto &entitiesLayer = level.getLayer("entities");
        auto &entities = entitiesLayer.allEntities();

        for (auto &entity : entities)
        {
            auto *node = CreateNode(this, entity);

            if (node == nullptr)
                continue;

            node->game = this;

            node->onCreated();

            if (node->name != "player")
                prependChild(node);
            else
                addChild(node);
        }

        uiNode->onCreated();
        addChild(uiNode);
    }

    bool getFlag(std::string flag)
    {
        return flags[flag];
    }

    bool setFlag(std::string flag, bool value)
    {
        flags[flag] = value;
        return value;
    }

    void drawOverlayDialog(float fElapsedTime)
    {
        if (dialogs.empty())
            return;

        auto currentDialog = &dialogs[0];
        currentDialog->duration -= fElapsedTime;

        if (currentDialog->duration <= 0 && !currentDialog->persistent)
        {
            dialogs.erase(dialogs.begin());
            return;
        }

        if (currentDialog->fullscreen)
        {
            Text(currentDialog->message, olc::WHITE, YAlign::MIDDLE, XAlign::CENTER, {1, 1});
            return;
        }

        auto textSize = TextSize(currentDialog->message);
        const auto rectColor = olc::Pixel(0, 0, 0, 150);

        Rect({10, 10}, {SCREEN_WIDTH - 20, textSize.y + 20.0f}, rectColor, true);
        Text(currentDialog->message, olc::WHITE, YAlign::TOP, XAlign::LEFT, {1, 1}, {20, 20});
    }

    void clearDialogs()
    {
        dialogs.clear();
    }

    bool isPersistentDialogOpen()
    {
        if (dialogs.empty())
            return false;

        return dialogs[0].persistent;
    }

    void popDialog()
    {
        if (dialogs.empty())
            return;

        dialogs.erase(dialogs.begin());
    }

    bool isFullscreenDialog()
    {
        if (dialogs.empty())
            return false;

        return dialogs[0].fullscreen;
    }

    void onUpdated(float fElapsedTime) override
    {
        if (isFullscreenDialog())
        {
            drawOverlayDialog(fElapsedTime);
            return;
        }

        // Drawing background image
        Image(backgroundProvider);
        updateOnScreenColliders();

        auto &world = project.getWorld();
        auto &level = world.getLevel(selectedLevel);

        // Adding tiles
        auto &platform = level.getLayer("platform");
        int skipped = 0;
        for (auto &tile : platform.allTiles())
        {
            auto rect = tile.getTextureRect();
            auto tileWorldPosition = tile.getPosition();

            if (!camera->IsOnScreen(tileWorldPosition))
            {
                skipped++;
                continue;
            }

            auto *options = new AssetOptions();
            olc::vf2d itemPosition = {static_cast<float>(tileWorldPosition.x), static_cast<float>(tileWorldPosition.y)};
            camera->WorldToScreen(itemPosition);

            options->position = itemPosition;
            options->offset = olc::vf2d{(float)rect.x, (float)rect.y};
            options->size = olc::vf2d{(float)rect.width, (float)rect.height};

            Image(spritesProvider, options);
        }

        CoreNode::onUpdated(fElapsedTime);
        drawOverlayDialog(fElapsedTime);

        if (isGameOver)
        {
            Text("Game Over", olc::WHITE, YAlign::MIDDLE, XAlign::CENTER, {2.0, 2.0});
            Text("Press ESC to restart", olc::WHITE, YAlign::MIDDLE, XAlign::CENTER, {1, 1}, {0, 20.0});
        }
    }

    bool isDisplayingDialog()
    {
        return !dialogs.empty();
    }

    bool isDisplayingDialogWithId(uint8_t id)
    {
        if (dialogs.empty())
            return false;

        return dialogs[0].id == id;
    }

    void addDialog(Dialog dialog)
    {
        if (dialog.duration <= 0 && !dialog.persistent || dialog.message.empty())
            return;

        for (auto &d : dialogs)
            if (d.id == dialog.id)
                return;

        dialogs.push_back(dialog);
    }

    void onUp() override
    {
        CoreNode::onUp();

        for (auto &child : children)
            child->onUp();
    }

    void onDown() override
    {
        CoreNode::onDown();

        for (auto &child : children)
            child->onDown();
    }

    void onLeft() override
    {
        CoreNode::onLeft();

        for (auto &child : children)
            child->onLeft();
    }

    void onRight() override
    {
        CoreNode::onRight();

        for (auto &child : children)
            child->onRight();
    }

    void onEnter() override
    {
        CoreNode::onEnter();

        if (isPersistentDialogOpen())
        {
            popDialog();
            return;
        }

        for (auto &child : children)
            child->onEnter();
    }

    void onGameOver()
    {
        isGameOver = true;
    }

    std::vector<olc::utils::geom2d::rect<float> *> &getOnScreenColliders()
    {
        return onScreenColliders;
    }

    template <typename T>
    T *getChild()
    {
        for (auto &child : children)
        {
            auto casted = dynamic_cast<T *>(child);
            if (casted != nullptr)
                return casted;
        }

        return nullptr;
    }

private:
    void updateOnScreenColliders()
    {
        onScreenColliders.clear();
        for (auto &collider : colliders)
        {
            if (camera->IsOnScreen(collider->pos))
            {
                onScreenColliders.push_back(collider);
            }
        }
    }
};
