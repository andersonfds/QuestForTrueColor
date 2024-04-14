class QuestForTrueColor;

Node *CreateEntity(const ldtk::Entity &entity);

enum GameOption
{
    PLAYGROUND,
    NEW_GAME,
    CREDITS,
};

class MainMenu : public Node
{
private:
    std::map<GameOption, std::string> options;
    olc::Pixel unselectedColor = olc::Pixel(255, 255, 255, 100);
    GameOption selectedOption = GameOption::NEW_GAME;
    Layer *gameLayer;

public:
    MainMenu(Layer *gameLayer)
    {
        this->gameLayer = gameLayer;
    }

    void OnCreate() override
    {
        options.clear();
        StopMusic();

        if (DEBUG)
            options[GameOption::PLAYGROUND] = "Playground";
        options[GameOption::NEW_GAME] = "New Game";
        options[GameOption::CREDITS] = "Credits";
    }

    void OnOptionSelected(GameOption option)
    {
        switch (option)
        {
        case GameOption::PLAYGROUND:
        {
            Map *map = new Map();
            gameLayer->AddNode(map);
            gameLayer->OnCreate();
            SetLevel("level_2", "Act 2. The Beginning");
            gameLayer->RemoveNode(this);

            break;
        }

        case GameOption::NEW_GAME:
        {
            Map *map = new Map();
            gameLayer->AddNode(map);
            gameLayer->OnCreate();
            SetLevel("level_1", "Act 1. The Beginning");
            gameLayer->RemoveNode(this);
            break;
        }

        case GameOption::CREDITS:
            break;
        }
    }

    void SetLevel(std::string level, std::string message)
    {
        Map *map = gameLayer->GetNode<Map>();
        map->SetActiveLevel(level, message);
    }

    void OnProcess(float fElapsedTime) override
    {
        GetEngine()->Clear(olc::VERY_DARK_BLUE);
        GetEngine()->DrawString(10, 10, "Quest for True Color", olc::WHITE, 2);
        int y = 50;
        std::string selectedOptionText = "";

        for (auto option : options)
        {
            bool isSelected = selectedOption == option.first;
            olc::Pixel color = isSelected ? olc::WHITE : unselectedColor;
            GetEngine()->DrawString(10, y, option.second, color, 2);
            y += 20;

            if (isSelected)
            {
                selectedOptionText = option.second;
            }
        }

        int selectedOptionIndex = 0;
        for (auto option : options)
        {
            if (option.second == selectedOptionText)
            {
                break;
            }
            selectedOptionIndex++;
        }

        // iterate through the options when keyboard up or down is pressed
        // and set the selected option
        if (GetEngine()->GetKey(olc::Key::DOWN).bPressed)
        {
            selectedOptionIndex++;
            if (selectedOptionIndex >= options.size())
            {
                selectedOptionIndex = 0;
            }
        }
        else if (GetEngine()->GetKey(olc::Key::UP).bPressed)
        {
            selectedOptionIndex--;
            if (selectedOptionIndex < 0)
            {
                selectedOptionIndex = options.size() - 1;
            }
        }

        auto it = options.begin();
        std::advance(it, selectedOptionIndex);
        selectedOption = it->first;

        if (GetEngine()->GetKey(olc::Key::ENTER).bPressed)
        {
            OnOptionSelected(selectedOption);
        }
    }
};