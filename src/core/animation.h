#pragma once

class AnimationController
{
private:
    std::vector<int> frames;
    int frame;
    float speed;
    float time = 0;

public:
    int tilesetId = 0;
    olc::vf2d position;

public:
    AnimationController(float speed = 0.1f, olc::vf2d position = {0.0f, 0.0f}, int initialFrame = 0, int frames = 1)
    {
        this->speed = speed;
        this->position = position;
        this->frame = initialFrame;

        for (int i = 0; i < frames; i++)
        {
            this->frames.push_back(i);
        }
    }

    AnimationController(float speed = 0.1f, olc::vf2d position = {0.0f, 0.0f}, std::vector<int> frames = {0})
    {
        this->speed = speed;
        this->position = position;
        this->frames = frames;
    }

    AnimationController(Node *node, float speed = 0.1f, int initialFrame = 0, std::vector<int> frames = {0})
    {
        auto map = node->GetLayer()->GetNode<Map>();
        auto &entity = map->GetEntity(node->GetEntityID());
        auto texture = entity.getTextureRect();
        this->tilesetId = map->GetTilesetIDByPath(entity.getTexturePath());
        this->position = {static_cast<float>(texture.x), static_cast<float>(texture.y)};
        this->speed = speed;
        this->frame = initialFrame;
        this->frames = frames;
    }

    olc::vf2d GetFrame(float fElapsedTime)
    {
        time += fElapsedTime;
        olc::vf2d framePos = position;
        int frameSize = frames.size();

        if (time > speed)
        {
            time = 0;
            if (++frame >= frameSize)
            {
                frame = 0;
            }
        }

        int currentFrame = frames[frame % frameSize];
        framePos.x += currentFrame * 32;

        return framePos;
    }
};
