class GameNode;
class AssetOptions;

using namespace olc::utils;

struct Camera
{
    olc::vf2d *size;
    olc::vf2d *position;
    olc::vf2d *offset;

    float zoom;

    Camera()
    {
        position = new olc::vf2d();
        size = new olc::vf2d();
        zoom = 1.000f;
        offset = new olc::vf2d();
    }

    void ScreenToWorld(olc::vf2d &screen)
    {
        auto position = GetPosition();
        screen.x = screen.x / zoom + position.x;
        screen.y = screen.y / zoom + position.y;

        screen.x -= offset->x;
        screen.y -= offset->y;
    }

    void WorldToScreen(olc::vf2d &world)
    {
        auto position = GetPosition();
        world.x = (world.x - position.x) * zoom;
        world.y = (world.y - position.y) * zoom;

        world.x += offset->x;
        world.y += offset->y;
    }

    bool IsOnScreen(olc::vf2d pos)
    {
        auto position = GetPosition();
        olc::vf2d screenSize = {SCREEN_WIDTH, SCREEN_HEIGHT};

        geom2d::rect<float> cameraRect = geom2d::rect<float>({position, screenSize});
        geom2d::rect<float> objectRect = geom2d::rect<float>(pos, {16, 16});

        // add some padding to the camera
        cameraRect.pos.x -= 40;
        cameraRect.pos.y -= 40;
        cameraRect.size.x += 40;
        cameraRect.size.y += 40;

        // account for the offset
        cameraRect.pos.x -= offset->x;
        cameraRect.pos.y -= offset->y;

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
        olc::vf2d position = *this->position;

        position.x -= offset->x;
        position.y -= offset->y;

        position.x = std::max(0.0f, std::min(position.x, size->x - screenSize.x));
        position.y = std::max(0.0f, std::min(position.y, size->y - screenSize.y));

        position.x += halfScreenSize.x;
        position.y += halfScreenSize.y;

        return position;
    }

    bool IsOfflimits(olc::vf2d pos)
    {
        return pos.y > size->y;
    }

    ~Camera()
    {
        delete position;
        delete size;
        delete offset;
    }
};

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
        for (auto child : children)
            delete child;
    }

    bool empty() const
    {
        return children.empty();
    }

    virtual bool addChild(CoreNode *node)
    {
        // check if child already exists
        if (std::find(children.begin(), children.end(), node) != children.end())
            return false;

        children.push_back(node);
        return true;
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

    virtual void onCreated()
    {
        clearChildren();
    }

    virtual void onUpdated(float fElapsedTime)
    {
        if (parent != nullptr)
            position = parent->position;

        for (auto child : children)
            child->onUpdated(fElapsedTime);
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
            newParent->addChild(child);
            child->onReparent();
        }

        clearChildren();
    }
};

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