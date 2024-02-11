#ifndef COLLISION_H
#define COLLISION_H

class box
{
public:
    olc::vf2d *position;
    int width, height;
    bool transparent;

    box(olc::vf2d *position, int width, int height, bool transparent = false)
    {
        this->position = position;
        this->width = width;
        this->height = height;
        this->transparent = transparent;
    }

    bool Intersects(box *other)
    {
        return this->position->x<other->position->x + other->width &&this->position->x + this->width> other->position->x &&
               this->position->y<other->position->y + other->height &&this->position->y + this->height> other->position->y;
    }
};

#endif