#pragma once

using namespace olc::utils::geom2d;

class GameNode;
class PlayerNode;

#pragma region Camera

struct Camera
{
    olc::vf2d size;
    olc::vf2d position;
    olc::vf2d offset;

    float zoom;

    Camera()
    {
        position = olc::vf2d();
        size = olc::vf2d();
        zoom = 1.000f;
        offset = olc::vf2d();
    }

    void ScreenToWorld(olc::vf2d &screen)
    {
        auto position = GetPosition();
        screen.x = screen.x / zoom + position.x;
        screen.y = screen.y / zoom + position.y;

        screen.x -= offset.x;
        screen.y -= offset.y;
    }

    void WorldToScreen(olc::vf2d &world)
    {
        auto position = GetPosition();
        world.x = (world.x - position.x) * zoom;
        world.y = (world.y - position.y) * zoom;

        world.x += offset.x;
        world.y += offset.y;
    }

    bool IsOnScreen(olc::vf2d pos)
    {
        auto position = GetPosition();
        olc::vf2d screenSize = {SCREEN_WIDTH, SCREEN_HEIGHT};

        rect<float> cameraRect = rect<float>({position, screenSize});
        rect<float> objectRect = rect<float>(pos, {16, 16});

        // add some padding to the camera
        cameraRect.pos.x -= 40;
        cameraRect.pos.y -= 40;
        cameraRect.size.x += 40;
        cameraRect.size.y += 40;

        // account for the offset
        cameraRect.pos.x -= offset.x;
        cameraRect.pos.y -= offset.y;

        return overlaps(cameraRect, objectRect);
    }

    bool IsOnScreen(const ldtk::IntPoint &point)
    {
        olc::vf2d pos = {static_cast<float>(point.x), static_cast<float>(point.y)};
        return IsOnScreen(pos);
    }

    olc::vf2d GetPosition()
    {
        olc::vf2d screenSize = {SCREEN_WIDTH, SCREEN_HEIGHT};
        olc::vf2d halfScreenSize = screenSize * 0.5f;
        olc::vf2d position = this->position;

        position.x -= offset.x;
        position.y -= offset.y;

        position.x = std::max(0.0f, std::min(position.x, size.x - screenSize.x));
        position.y = std::max(0.0f, std::min(position.y, size.y - screenSize.y));

        position.x += halfScreenSize.x;
        position.y += halfScreenSize.y;

        return position;
    }

    bool IsOfflimits(olc::vf2d pos)
    {
        return pos.y > size.y;
    }
};

#pragma endregion Camera

#pragma region CoreNode

class CoreNode
{
public:
    std::string name;
    olc::vf2d position;
    GameNode *game = nullptr;
    CoreNode *parent = nullptr;
    std::vector<CoreNode *> children;
    AssetOptions *thumbnail = nullptr;

    CoreNode(const std::string &name, GameNode *game) : name(name), position({0, 0}), game(game)
    {
    }

    virtual ~CoreNode()
    {
    }

    bool empty() const
    {
        return children.empty();
    }

    virtual rect<float> getCollider()
    {
        return rect<float>(position, {SPRITE_SIZE, SPRITE_SIZE});
    }

    virtual bool addChild(CoreNode *node)
    {
        // check if child already exists
        if (std::find(children.begin(), children.end(), node) != children.end())
            return false;

        children.push_back(node);
        return true;
    }

    virtual void onMiniGameOver(std::string gameName, bool didWin)
    {
    }

    bool prependChild(CoreNode *node)
    {
        // check if child already exists
        if (std::find(children.begin(), children.end(), node) != children.end())
            return false;

        children.insert(children.begin(), node);
        return true;
    }

    void removeChild(CoreNode *node)
    {
        children.erase(std::remove(children.begin(), children.end(), node), children.end());
    }

    void moveChildrenToRoot(CoreNode *root)
    {
        reparentChildren(root, true);
    }

    void moveChildToRoot(CoreNode *child, CoreNode *root)
    {
        if (child->parent != this)
            return;

        child->parent = nullptr;
        root->addChild(child);
        child->onReparent();
        removeChild(child);
    }

    void clearChildren()
    {
        children.clear();
    }

    void reparent(CoreNode *newParent)
    {
        if (parent)
            parent->removeChild(this);

        parent = newParent;
        newParent->addChild(this);
    }

    template <typename T>
    T *getChildOfType(std::string name = "")
    {
        for (auto &child : children)
        {
            if (!name.empty() && child->name != name)
                continue;
            auto casted = dynamic_cast<T *>(child);
            if (casted != nullptr)
                return casted;
        }

        return nullptr;
    }

    virtual void onCreated()
    {
        clearChildren();
    }

    virtual void onAllCreated()
    {
    }

    virtual void onUpdated(float fElapsedTime)
    {
        if (parent != nullptr)
            position = parent->position;

        for (auto child : children)
            child->onUpdated(fElapsedTime);
    }

    template <typename T>
    int getFirstIndexOfType()
    {
        for (int i = 0; i < children.size(); i++)
        {
            auto casted = dynamic_cast<T *>(children[i]);
            if (casted != nullptr)
                return i;
        }

        return -1;
    }

    virtual void onUp()
    {
    }

    virtual void onDown()
    {
    }

    virtual void onLeft()
    {
    }

    virtual void onRight()
    {
    }

    virtual void onEnter()
    {
    }

    virtual void onReparent()
    {
    }

private:
    void reparentChildren(CoreNode *newParent, bool clearParent = false)
    {
        for (auto child : children)
        {
            child->parent = clearParent ? nullptr : newParent;
            newParent->prependChild(child);
            child->onReparent();
        }

        clearChildren();
    }
};

#pragma endregion CoreNode

#pragma region MiniGame

class MiniGame : public CoreNode
{
private:
    bool isGameOver = false;
    bool isGameWon = false;

public:
    MiniGame(const std::string &name, GameNode *game) : CoreNode(name, game)
    {
    }

    void finishGame(bool won)
    {
        isGameOver = true;
        isGameWon = won;
    }

    bool getIsGameWon()
    {
        return isGameWon;
    }

    bool getIsGameOver()
    {
        return isGameOver;
    }

    void onCreated() override
    {
        CoreNode::onCreated();
        isGameOver = false;
        isGameWon = false;
    }

    void onUpdated(float fElapsedTime) override
    {
        CoreNode::onUpdated(fElapsedTime);

        if (isGameOver)
            return;

        for (auto &child : children)
            child->onUpdated(fElapsedTime);
    }
};

#pragma endregion MiniGame

#pragma region Iterator

class CoreNodeIterator
{
private:
    std::stack<CoreNode *> nodes;

public:
    CoreNodeIterator(CoreNode *root)
    {
        if (root)
            nodes.push(root);
    }

    bool hasNext()
    {
        return !nodes.empty();
    }

    CoreNode *next()
    {
        if (nodes.empty())
            return nullptr;

        CoreNode *current = nodes.top();
        nodes.pop();

        for (auto it = current->children.rbegin(); it != current->children.rend(); ++it)
        {
            if (*it)
                nodes.push(*it);
        }

        return current;
    }
};

enum class HAlign
{
    Center,
    Left,
    Right,
};

enum class VAlign
{
    Center,
    Top,
    Bottom,
};

#pragma endregion Iterator

#pragma region GameNode

CoreNode *CreateNode(GameNode *node, const ldtk::Entity &entity);
MiniGame *CreateMiniGame(const std::string &name, GameNode *node);

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
    Sound *deadSound;
    CoreNode *playerNode;
    MiniGame *currentMiniGame;
    bool displayingMinigame = false;
    bool didLoadMusic = false;

public:
    bool isGameOver = false;
    Camera camera;
    GameImageAssetProvider *spritesProvider;
    CoreNode *uiNode;

    GameNode(CoreNode *uiNode) : CoreNode("Game", nullptr)
    {
        onScreenColliders.reserve(100);
        dialogs.reserve(10);
        this->uiNode = uiNode;
    }

    const ldtk::Enum &getGameEnum(const std::string &name)
    {
        return project.getWorld().getEnum(name);
    }

    olc::vf2d getPositionForEnumValue(const std::string &enumName, const std::string &value)
    {
        auto &enumValue = getGameEnum(enumName)[value];
        auto &textureRect = enumValue.getIconTextureRect();
        return {static_cast<float>(textureRect.x), static_cast<float>(textureRect.y)};
    }

    void onCreated() override
    {
        CoreNode::onCreated();
        selectedLevel = "level_1";
        camera = Camera();
        project.loadFromFile("assets/map_project/QuestForTrueColor.ldtk");
        spritesProvider = new GameImageAssetProvider("assets/sprite_project/Sprites.png");
        deadSound = new Sound("assets/sfx/game_over.wav", 1);
        loadLevel(selectedLevel);
        displayingMinigame = false;
        addDialog({"Developed by Anderson, with love and coffee <3", 3.0f, true, false});
        didLoadMusic = false;
    }

    ~GameNode()
    {
        delete spritesProvider;
        delete deadSound;
        delete backgroundProvider;
        ClearMusic();
    }

    void setMiniGame(const std::string &game = "ShellGame")
    {
        if (currentMiniGame != nullptr)
            delete currentMiniGame;

        currentMiniGame = CreateMiniGame(game, this);

        if (currentMiniGame != nullptr)
        {
            displayingMinigame = true;
            currentMiniGame->onCreated();
        }
    }

    bool isMiniGameActive()
    {
        if (currentMiniGame == nullptr)
            return false;

        bool isGameOver = currentMiniGame->getIsGameOver();

        if (isGameOver)
        {
            auto gameName = currentMiniGame->name;
            auto didWin = currentMiniGame->getIsGameWon();
            for (auto &child : children)
                child->onMiniGameOver(gameName, didWin);

            delete currentMiniGame;
            currentMiniGame = nullptr;
            displayingMinigame = false;
        }

        return !isGameOver;
    }

    void loadLevel(const std::string levelName)
    {
        auto &allLevels = project.getWorld().allLevels();
        bool didFind = false;
        for (auto &level : allLevels)
            if (level.name == levelName)
            {
                didFind = true;
                break;
            }

        if (!didFind)
            return;

        selectedLevel = levelName;
        colliders.clear();
        clearDialogs();
        disableLevelPortal();
        clearChildren();
        auto &world = project.getWorld();
        auto &level = world.getLevel(selectedLevel);
        auto &bgImage = level.getBgImage();
        const auto bgImagePath = bgImage.path.c_str();
        auto fullImagePath = "assets/map_project/" + std::string(bgImagePath);
        backgroundProvider = new GameImageAssetProvider(fullImagePath);

        camera.size.x = level.size.x;
        camera.size.y = level.size.y;
        isGameOver = false;
        deadSound->SetPlayed(false);

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

            if (node->name == "player")
                playerNode = node;

            addChild(node);
        }

        uiNode->onCreated();
        addChild(uiNode);

        for (auto &child : children)
            child->onAllCreated();
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
        if (!IsPlayingMusic() && !didLoadMusic)
        {
            LoadMusic("assets/sfx/huperboloid.wav");
            didLoadMusic = true;
        }

        if (isFullscreenDialog())
        {
            drawOverlayDialog(fElapsedTime);
            StopMusic();
            return;
        }

        if (didLoadMusic)
            ResumeMusic();

        // Drawing background image
        Image(backgroundProvider);

        if (isMiniGameActive())
        {
            currentMiniGame->onUpdated(fElapsedTime);
            drawOverlayDialog(fElapsedTime);
            return;
        }

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

            if (!camera.IsOnScreen(tileWorldPosition))
            {
                skipped++;
                continue;
            }

            auto *options = new AssetOptions();
            olc::vf2d itemPosition = {static_cast<float>(tileWorldPosition.x), static_cast<float>(tileWorldPosition.y)};
            camera.WorldToScreen(itemPosition);

            options->position = itemPosition;
            options->offset = olc::vf2d{(float)rect.x, (float)rect.y};
            options->size = olc::vf2d{(float)rect.width, (float)rect.height};

            Image(spritesProvider, options);
        }

        // Drawing entities, player behind everything
        if (playerNode != nullptr)
            playerNode->onUpdated(fElapsedTime);

        for (auto &child : children)
        {
            if (child == playerNode)
                continue;

            child->onUpdated(fElapsedTime);
        }

        drawOverlayDialog(fElapsedTime);
        if (isGameOver)
        {
            Text("Game Over", olc::WHITE, YAlign::MIDDLE, XAlign::CENTER, {2.0, 2.0});
            Text("Press ESC to restart", olc::WHITE, YAlign::MIDDLE, XAlign::CENTER, {1, 1}, {0, 20.0});
            deadSound->Play(false, false);
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

    void enableLevelPortal()
    {
        this->setFlag("level_portal", true);
    }

    void disableLevelPortal()
    {
        this->setFlag("level_portal", false);
    }

    bool isLevelPortalEnabled()
    {
        return this->getFlag("level_portal");
    }

    bool hasPersistentDialogShowing()
    {
        if (dialogs.empty())
            return false;

        return dialogs[0].persistent;
    }

    bool hasPendingDialogWithId(uint8_t id)
    {
        if (dialogs.empty())
            return false;

        for (auto &dialog : dialogs)
            if (dialog.id == id && (dialog.duration > 0 || dialog.persistent))
                return true;

        return false;
    }

    template <typename T>
    std::vector<T *> getOnScreenChildrenOfType(bool evaluateScreen = true)
    {
        std::vector<T *> output;
        for (auto &child : children)
        {
            if (!camera.IsOnScreen(child->position) && evaluateScreen)
                continue;

            auto eval = dynamic_cast<T *>(child);
            if (eval != nullptr)
                output.push_back(eval);
        }

        return output;
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
            if (camera.IsOnScreen(collider->pos))
            {
                onScreenColliders.push_back(collider);
            }
        }
    }
};

#pragma endregion GameNode

#pragma region Core Entity

class EntityNode : public CoreNode
{
protected:
    const ldtk::Entity &entity;
    Camera *camera;
    GameImageAssetProvider *spritesProvider;

public:
    EntityNode(const ldtk::Entity &entity, GameNode *game) : CoreNode(entity.getName(), game), entity(entity)
    {
        if (game != nullptr)
        {
            camera = &game->camera;
            spritesProvider = game->spritesProvider;
        }
    }

    olc::vi2d getSpriteDrawPosition()
    {
        auto &textureRect = entity.getTextureRect();
        return olc::vi2d{textureRect.x, textureRect.y};
    }

    void onCreated() override
    {
        CoreNode::onCreated();

        auto &entityPos = entity.getPosition();
        position = {(float)entityPos.x, (float)entityPos.y};
    }

    void onUpdated(float fElapsedTime) override
    {
        CoreNode::onUpdated(fElapsedTime);
        if (DEBUG)
        {
            // Draw collider
            auto collider = getCollider();
            auto pos = collider.pos;
            game->camera.WorldToScreen(pos);
            Rect(pos, collider.size, olc::RED);
        }
    }
};

#pragma endregion Core Entity
