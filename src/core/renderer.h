#pragma once

class Layer;

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
        geom2d::rect<float> cameraRect = geom2d::rect<float>({position, *size});
        geom2d::rect<float> objectRect = geom2d::rect<float>(pos, {16, 16});

        // add some padding to the camera
        cameraRect.pos.x -= 100;
        cameraRect.pos.y -= 100;
        cameraRect.size.x += 100;
        cameraRect.size.y += 100;

        // account for the offset
        cameraRect.pos.x -= offset->x;
        cameraRect.pos.y -= offset->y;

        return overlaps(cameraRect, objectRect);
    }

    olc::vf2d GetPosition()
    {
        olc::vf2d screenSize = {SCREEN_WIDTH, SCREEN_HEIGHT};
        olc::vf2d halfScreenSize = screenSize / 2;
        olc::vf2d position = *this->position;

        position.x -= offset->x;
        position.y -= offset->y;

        position.x = std::max(0.0f, std::min(position.x, size->x - screenSize.x));
        position.y = std::max(0.0f, std::min(position.y, size->y - screenSize.y));

        position.x += halfScreenSize.x;
        position.y += halfScreenSize.y;

        return position;
    }
};

class Node
{
private:
    olc::PixelGameEngine *pge;
    Camera *camera;
    Layer *layer;
    ldtk::IID entityID;

protected:
    /**
     * @brief Get the engine object
     *
     * @return olc::PixelGameEngine*
     */
    olc::PixelGameEngine *GetEngine()
    {
        return pge;
    }

    /**
     * @brief Get the Key object
     *
     * @param key
     * @return olc::Key
     */
    olc::HWButton GetKey(olc::Key key)
    {
        return pge->GetKey(key);
    }

    bool IsDebug()
    {
        return DEBUG;
    }

    Camera *GetCamera()
    {
        return camera;
    }

public:
    /**
     * @brief Called when the node is created
     */
    virtual void OnCreate() {}

    /**
     * @brief Called every frame to draw the node
     */
    virtual void OnProcess(float fElapsedTime) {}

    /**
     * @brief Called every frame to draw the node
     */
    virtual void OnPhysicsProcess(float fElapsedTime) {}

    /**
     * @brief Called when the node is destroyed
     */
    virtual void OnDestroy() {}

    /**
     * @brief Set the Engine object
     *
     * @param pge
     */
    void SetEngine(olc::PixelGameEngine *pge)
    {
        this->pge = pge;
    }

    void SetCamera(Camera *camera)
    {
        this->camera = camera;
    }

    void SetLayer(Layer *layer)
    {
        this->layer = layer;
    }

    void SetEntityID(ldtk::IID entityID)
    {
        this->entityID = entityID;
    }

    Layer *GetLayer()
    {
        return layer;
    }

    ldtk::IID GetEntityID()
    {
        return entityID;
    }
};

class Layer
{
public:
    Layer(std::string name, olc::PixelGameEngine *pge)
    {
        this->name = name;
        this->pge = pge;
        this->camera = new Camera();
    }

    std::string name;

    void OnCreate()
    {
        for (auto node : nodes)
        {
            node->OnCreate();
        }
    }

    void AddNode(Node *node)
    {
        node->SetEngine(pge);
        node->SetCamera(camera);
        node->SetLayer(this);
        nodes.push_back(node);
    }

    void RemoveNode(Node *node)
    {
        nodes.erase(std::remove(nodes.begin(), nodes.end(), node), nodes.end());
    }

    void Process(float fElapsedTime)
    {
        for (auto node : nodes)
        {
            node->OnProcess(fElapsedTime);
        }
    }

    void PhysicsProcess(float fElapsedTime)
    {
        for (auto node : nodes)
        {
            node->OnPhysicsProcess(fElapsedTime);
        }
    }

    olc::PixelGameEngine *GetEngine()
    {
        return pge;
    }

    void SetCameraPosition(olc::vf2d position)
    {
        auto screenWidth = pge->ScreenWidth();
        auto screenHeight = pge->ScreenHeight();

        camera->position->x = std::max(0.0f, std::min(position.x, camera->size->x - screenWidth));
        camera->position->y = std::max(0.0f, std::min(position.y, camera->size->y - screenHeight));
    }

    olc::vf2d GetCameraPosition()
    {
        return *camera->position;
    }

    Camera *GetCamera()
    {
        return camera;
    }

    template <typename T>
    T *GetNode()
    {
        for (auto node : nodes)
        {
            T *t = dynamic_cast<T *>(node);
            if (t)
            {
                return t;
            }
        }

        return nullptr;
    }

private:
    std::vector<Node *> nodes;
    olc::PixelGameEngine *pge;
    Camera *camera;
};