#pragma once

class Animation
{
public:
    Animation(float speed, std::vector<std::string> paths)
    {
        this->speed = speed;
        for (auto path : paths)
        {
            frames.push_back(std::make_unique<olc::Decal>(new olc::Sprite(path)));
        }
    }

    olc::Decal *GetFrame(float fElapsedTime)
    {
        time += fElapsedTime;

        if (time > speed)
        {
            time = 0;
        }

        return frames[(int)(time / speed * frames.size())].get();
    }

private:
    std::vector<std::unique_ptr<olc::Decal>> frames;
    float speed;
    float time = 0;
};
