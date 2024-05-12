#pragma once

struct GameImageAssetProvider
{
    olc::Decal *decal = nullptr;

    GameImageAssetProvider(std::string path)
    {
        this->decal = new olc::Decal(new olc::Sprite(path));
    }

    ~GameImageAssetProvider()
    {
        delete decal;
    }
};

struct AssetOptions
{
    olc::vf2d position;
    olc::vi2d offset;
    olc::vf2d scale;
    olc::vf2d size;
    olc::Pixel tint;

    AssetOptions(olc::vf2d position = {0, 0}, olc::vi2d offset = {0, 0}, olc::vf2d scale = {1, 1}, olc::vf2d size = {SPRITE_SIZE, SPRITE_SIZE}, olc::Pixel tint = olc::WHITE)
    {
        this->position = position;
        this->scale = scale;
        this->offset = offset;
        this->size = size;
        this->tint = tint;
    }

    AssetOptions *Copy()
    {
        return new AssetOptions(position, offset, scale, size, tint);
    }
};

class AnimatedAssetProvider
{

private:
    std::map<std::string, std::vector<olc::vf2d>> animations;
    std::map<std::string, float> animationSpeeds;
    olc::vf2d initialOffset;
    olc::vf2d position;
    olc::vf2d scale;
    olc::vf2d size;
    std::string currentAnimation = "";
    float elapsedTime = 0;
    AssetOptions *options = nullptr;

public:
    AnimatedAssetProvider(olc::vf2d position, olc::vf2d initialOffset, olc::vf2d scale = {1.0f, 1.0f}, olc::vf2d size = {SPRITE_SIZE, SPRITE_SIZE}, olc::Pixel tint = olc::WHITE)
    {
        this->initialOffset = initialOffset;
        this->position = position;
        this->scale = scale;
        this->size = size;

        options = new AssetOptions(position, initialOffset, scale, size, tint);
    }

    void AddAnimation(std::string name, float fps, std::vector<olc::vf2d> frames)
    {
        animations[name] = frames;
        animationSpeeds[name] = fps;
    }

    void PlayAnimation(std::string name, bool reset = true)
    {
        currentAnimation = name;
        if (reset)
            elapsedTime = 0;
    }

    void setTint(olc::Pixel tint, float opacity = 1.0f)
    {
        if (opacity < 1.0f && opacity > 0.0f)
        {
            tint.a = (uint8_t)(255 * opacity);
        }

        options->tint = tint;
    }

    void Update(float fElapsedTime)
    {
        if (currentAnimation == "")
        {
            return;
        }

        elapsedTime += fElapsedTime;

        auto fps = animationSpeeds[currentAnimation];
        auto frames = animations[currentAnimation].size();

        if (elapsedTime * fps >= frames)
        {
            elapsedTime = 0;
        }
    }

    AssetOptions *GetAssetOptions()
    {
        if (currentAnimation == "")
        {
            return options;
        }

        auto frame = animations[currentAnimation][(int)(elapsedTime * animationSpeeds[currentAnimation])];
        options->offset = initialOffset + frame * size;

        return options;
    }

    ~AnimatedAssetProvider()
    {
        delete options;
    }
};

enum class YAlign
{
    TOP,
    MIDDLE,
    BOTTOM
};

enum class XAlign
{
    LEFT,
    CENTER,
    RIGHT
};

/**
 * @brief Text
 * Draw text on the screen.
 *
 * @param data Text to draw.
 * @param color Color of the text.
 * @param yAlign Vertical alignment.
 * @param xAlign Horizontal alignment.
 * @param scale Scale of the text.
 * @param offset Offset of the text.
 */
void Text(std::string data, olc::Pixel color = olc::WHITE, YAlign yAlign = YAlign::TOP, XAlign xAlign = XAlign::LEFT, olc::vf2d scale = {1, 1}, olc::vf2d offset = {0, 0});

/**
 * @brief Text
 * Draw text on the screen.
 *
 * @param data Text to draw.
 * @param color Color of the text.
 * @param position Position of the text.
 * @param scale Scale of the text.
 */
void Text(std::string data, olc::Pixel color, olc::vf2d position, olc::vf2d scale = {1, 1});

/**
 * @brief TextSize
 * Get the size of the text to be drawn.
 */
olc::vi2d TextSize(std::string data);

void Image(GameImageAssetProvider *asset, AssetOptions *options = nullptr);

/**
 * @brief Rect
 * Draw a rectangle on the screen.
 *
 * @param position Position of the rectangle.
 * @param size Size of the rectangle.
 * @param color Color of the rectangle.
 * @param filled Fill the rectangle.
 */
void Rect(olc::vf2d position, olc::vf2d size, olc::Pixel color = olc::WHITE, bool filled = false);

/**
 * Whether a key is pressed.
 */
bool Pressed(olc::Key key);

/**
 * Whether a key is pressed.
 */
bool Pressed(int key);

/**
 * @brief MousePosition
 * Get the mouse position.
 *
 * @return const olc::vi2d&
 */
const olc::vi2d &MousePosition();

#ifdef USE_PIXEL_GAME_ENGINE

olc::PixelGameEngine *ctx = nullptr;

void SetContext(olc::PixelGameEngine *pge)
{
    ctx = pge;
}

olc::vi2d TextSize(std::string data)
{
    return ctx->GetTextSize(data);
}

void Text(std::string data, olc::Pixel color, YAlign yAlign, XAlign xAlign, olc::vf2d scale, olc::vf2d offset)
{
    float_t x = 0;
    float_t y = 0;
    auto screenSize = ctx->GetScreenSize();
    auto textSize = ctx->GetTextSize(data) * scale;

    switch (yAlign)
    {
    case YAlign::TOP:
        y = 0;
        break;
    case YAlign::MIDDLE:
        y = screenSize.y * 0.5f - textSize.y * 0.5f;
        break;
    case YAlign::BOTTOM:
        y = screenSize.y - textSize.y;
        break;
    }

    switch (xAlign)
    {
    case XAlign::LEFT:
        x = 0;
        break;
    case XAlign::CENTER:
        x = screenSize.x * 0.5f - textSize.x * 0.5f;
        break;
    case XAlign::RIGHT:
        x = screenSize.x - textSize.x;
        break;
    }

    x += offset.x;
    y += offset.y;

    ctx->DrawStringDecal({x, y}, data, color, scale);
}

void Text(std::string data, olc::Pixel color, olc::vf2d position, olc::vf2d scale)
{
    ctx->DrawStringDecal(position, data, color, scale);
}

void Image(GameImageAssetProvider *asset, AssetOptions *option)
{
    if (!asset || !asset->decal)
    {
        throw std::runtime_error("Asset or Decal is null");
        return;
    }

    olc::Decal *decal = asset->decal;

    if (!option)
    {
        ctx->DrawDecal({0, 0}, decal, {1, 1});
        return;
    }

    auto position = option->position;
    auto scale = option->scale;
    auto offset = option->offset;
    auto size = option->size;

    ctx->DrawPartialDecal(position, decal, offset, size, scale, option->tint);
}

void Rect(olc::vf2d position, olc::vf2d size, olc::Pixel color, bool filled)
{
    if (filled)
    {
        ctx->FillRectDecal(position, size, color);
    }
    else
    {
        ctx->DrawRectDecal(position, size, color);
    }
}

bool Pressed(olc::Key key)
{
    return ctx->GetKey(key).bPressed;
}

bool Pressed(int key)
{
    auto keyNumber = static_cast<olc::Key>(key);
    return ctx->GetKey(keyNumber).bPressed;
}

bool Held(olc::Key key)
{
    return ctx->GetKey(key).bHeld;
}

const olc::vi2d &MousePosition()
{
    return ctx->GetMousePos();
}

#endif
