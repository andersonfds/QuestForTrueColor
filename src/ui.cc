
class UINode : public CoreNode
{
private:
    AssetOptions *options;
    AssetOptions *slotOptions;
    float coinDeltaTime = 0.0f;
    uint8_t previousCoins = 0;

public:
    UINode(GameNode *game) : CoreNode("UI", game)
    {
    }

    ~UINode() override
    {
        delete options;
        delete slotOptions;
    }

    void onCreated() override
    {
        CoreNode::onCreated();
        this->options = getOptions("base", {16, 16});
        this->slotOptions = getOptions("slot", {SPRITE_SIZE, SPRITE_SIZE});
    }

    void onUpdated(float fElapsedTime) override
    {
        auto *player = getPlayer();

        if (!player)
        {
            return;
        }

        drawCoins(player->getMoney(), fElapsedTime);
        drawLives(player->getLives());
        drawStorage(player);
    }

    AssetOptions *getOptions(const std::string &name, olc::vi2d size = {SPRITE_SIZE, SPRITE_SIZE})
    {
        auto &uiOptions = game->getGameEnum("world");
        auto &baseEnumValue = uiOptions[name];
        auto &textureRect = baseEnumValue.getIconTextureRect();
        return new AssetOptions({0, 0}, {textureRect.x, textureRect.y}, {1, 1}, size);
    }

private:
    void drawCoins(uint8_t coins, float fElapsedTime = 0.0f)
    {
        // Draw coins to the left bottom corner
        auto *coinsOptions = options->Copy();
        if (coins != previousCoins)
        {
            coinDeltaTime = 0.0f;
            previousCoins = coins;
        }

        if (coinDeltaTime < 0.2f)
            coinDeltaTime += fElapsedTime;

        // interpolate the coinDeltaTime value so we can get a value between 1.0 and 0.6
        const float value = 0.6 + 0.4 * coinDeltaTime / 0.2;
        coinsOptions->scale = {value, value};

        coinsOptions->position = {10, SCREEN_HEIGHT - coinsOptions->size.y - 10};

        // applying to the position the scale so it can be centered
        coinsOptions->position += coinsOptions->size * 0.5 * (1 - value);

        auto coinsText = std::to_string(coins);
        Image(game->spritesProvider, coinsOptions);
        Text(coinsText, olc::WHITE, YAlign::BOTTOM, XAlign::LEFT, {1.5, 1.5}, {36, -12});
        delete coinsOptions;
    }

    void drawStorage(PlayerNode *player)
    {
        auto storage = player->getStorage();

        if (storage <= 1)
        {
            return;
        }

        // Draw storage items to the bottom center
        auto childCount = player->children.size();
        auto *storageOptions = slotOptions->Copy();

        // Computing the position of the first item
        float posX = SCREEN_WIDTH * 0.5 - (storage * SPRITE_SIZE * 0.5);
        float posY = SCREEN_HEIGHT - SPRITE_SIZE - 10;

        storageOptions->position = {posX, posY};

        for (int i = 0; i < storage; i++)
        {
            storageOptions->offset.y = slotOptions->offset.y;
            storageOptions->offset.x = slotOptions->offset.x;

            storageOptions->scale = {1, 1};
            bool isSelected = player->getSelectedIndex() == i;

            if (isSelected)
                storageOptions->offset.x += storageOptions->size.x;

            Image(game->spritesProvider, storageOptions);

            if (i < childCount)
            {
                auto *child = player->children[i];

                if (child->thumbnail)
                {
                    auto *storageChildOptions = child->thumbnail->Copy();
                    storageChildOptions->position = storageOptions->position;
                    Image(game->spritesProvider, storageChildOptions);
                    delete storageChildOptions;
                }
            }
            else
            {
                const olc::vf2d scale = olc::vf2d{1, 1} * (isSelected ? 1.2f : 0.8f);
                const auto textCenter = TextSize(std::to_string(i + 1)) * 0.5f * scale;
                const auto textPosition = storageOptions->position + olc::vf2d{SPRITE_SIZE * 0.5f, SPRITE_SIZE * 0.5f} - textCenter;

                // color semitransparent when not selected
                auto color = olc::WHITE;

                if (!isSelected)
                {
                    color.a = 100;
                }

                Text(std::to_string(i + 1), color, textPosition, scale);
            }

            storageOptions->position.x += storageOptions->size.x;
        }

        delete storageOptions;
    }

    void drawLives(uint8_t lives)
    {
        // Draw lives to the left bottom corner
        auto *livesOptions = options->Copy();
        livesOptions->offset.x += 16;
        livesOptions->position = {SCREEN_WIDTH - 10, SCREEN_HEIGHT - livesOptions->size.y - 10};
        const float factor = livesOptions->size.x * 1.15;

        for (int i = 0; i < lives; i++)
        {
            livesOptions->position.x -= factor;
            Image(game->spritesProvider, livesOptions);
        }

        delete livesOptions;
    }

    PlayerNode *getPlayer()
    {
        return game->getChild<PlayerNode>();
    }
};