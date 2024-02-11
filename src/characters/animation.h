#ifndef ANIMATION_H
#define ANIMATION_H

#include <vector>

class animation
{
public:
    animation(std::vector<std::string> frames, float fps = 0.1f)
    {
        this->fps = fps;
        for (auto frame : frames)
        {
            this->frames.push_back(new olc::Sprite(frame));
        }
    }

    ~animation()
    {
        for (auto frame : frames)
        {
            delete frame;
        }
    }

    olc::Sprite *getCurrentFrame(float deltaTime)
    {
        time += deltaTime;
        if (time >= fps)
        {
            time = 0;
            currentFrame++;
            if (currentFrame >= frames.size())
            {
                currentFrame = 0;
            }
        }
        return frames[currentFrame];
    }

private:
    std::vector<olc::Sprite *> frames;
    int currentFrame = 0;
    float fps, time = 0;
};

#endif