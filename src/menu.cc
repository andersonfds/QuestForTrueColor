
class MenuNode : public CoreNode
{
public:
    float deltaTime = 0.0f;
    GameImageAssetProvider *spritesProvider;
    AssetOptions *options;
    bool canContinue = false;

    std::vector<std::string> menuOptions = {"Continue", "New Game", "Exit"};
    std::string selectedOption = menuOptions[1];

    MenuNode() : CoreNode("menu", nullptr)
    {
    }

    bool canContinueGame()
    {
        return canContinue && !game->isGameOver;
    }

    void onCreated()
    {
        spritesProvider = new GameImageAssetProvider("assets/sprite_project/Sprites.png");
        options = new AssetOptions({0, 0}, {0, 0});
    }

    void onUpdated(float fElapsedTime)
    {
        olc::vf2d scale = {1.75f, 1.75f};

        if (deltaTime < 0.2f)
            deltaTime += fElapsedTime;

        auto menuOptions = this->menuOptions;

        if (!canContinueGame())
        {
            menuOptions.erase(std::remove(menuOptions.begin(), menuOptions.end(), "Continue"), menuOptions.end());
        }

        // interpolate the value so we can get a value between 0.6 and 1.0
        auto value = 0.6 + 0.4 * deltaTime / 0.2;

        auto topPadding = menuOptions.size() * TextSize("A") * scale;

        for (int i = 0; i < menuOptions.size(); i++)
        {
            bool isSelected = menuOptions[i] == selectedOption;
            auto textSize = TextSize(menuOptions[i]) * scale;
            auto offset = olc::vf2d{0, 0};
            offset.y = i * textSize.y - topPadding.y * 0.5f;
            olc::Pixel color = isSelected ? olc::YELLOW : olc::WHITE;

            Text(menuOptions[i], color, YAlign::MIDDLE, XAlign::CENTER, scale * (isSelected ? 1.0 * value : 0.6), offset);
        }
    }

    void onUp()
    {
        deltaTime = 0.0f;

        auto menuOptions = this->menuOptions;
        if (!canContinueGame())
        {
            menuOptions.erase(std::remove(menuOptions.begin(), menuOptions.end(), "Continue"), menuOptions.end());
        }

        if (selectedOption.empty())
        {
            selectedOption = menuOptions[0];
        }
        else
        {
            auto index = std::find(menuOptions.begin(), menuOptions.end(), selectedOption) - menuOptions.begin();
            index = (index - 1) % menuOptions.size();
            selectedOption = menuOptions[index];
        }
    }

    void onDown()
    {
        deltaTime = 0.0f;

        auto menuOptions = this->menuOptions;
        if (!canContinueGame())
        {
            menuOptions.erase(std::remove(menuOptions.begin(), menuOptions.end(), "Continue"), menuOptions.end());
        }

        if (selectedOption.empty())
        {
            selectedOption = menuOptions[0];
        }
        else
        {
            auto index = std::find(menuOptions.begin(), menuOptions.end(), selectedOption) - menuOptions.begin();
            index = (index + 1) % menuOptions.size();
            selectedOption = menuOptions[index];
        }
    }
};