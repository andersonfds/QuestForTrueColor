#pragma once

class Node
{
private:
    olc::PixelGameEngine *pge;

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
};

class Layer
{
public:
    Layer(std::string name, olc::PixelGameEngine *pge)
    {
        this->name = name;
        this->pge = pge;
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

private:
    std::vector<Node *> nodes;
    olc::PixelGameEngine *pge;
};
