#pragma once

struct Camera
{
    olc::vf2d *size;
    olc::vf2d *position;
    float zoom;

    Camera()
    {
        position = new olc::vf2d();
        size = new olc::vf2d();
        zoom = 1.000f;
    }

    void ScreenToWorld(olc::vf2d &screen)
    {
        screen.x = screen.x / zoom + position->x;
        screen.y = screen.y / zoom + position->y;
    }

    void WorldToScreen(olc::vf2d &world)
    {
        world.x = (world.x - position->x) * zoom;
        world.y = (world.y - position->y) * zoom;
    }
};

class Node
{
private:
    olc::PixelGameEngine *pge;
    Camera *camera;

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

private:
    std::vector<Node *> nodes;
    olc::PixelGameEngine *pge;
    Camera *camera;
};