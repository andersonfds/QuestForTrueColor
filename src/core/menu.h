class QuestForTrueColor;

Node *CreateEntity(const ldtk::Entity &entity);

enum GameOption
{
    PLAYGROUND,
    NEW_GAME,
    CREDITS
};

class MainMenu : public Node
{
private:
    std::map<GameOption, std::string> options;
    olc::Pixel unselectedColor = olc::Pixel(255, 255, 255, 100);
    GameOption selectedOption = GameOption::PLAYGROUND;
    Layer *gameLayer;

public:
    MainMenu(Layer *gameLayer)
    {
        this->gameLayer = gameLayer;
    }

    void OnCreate() override
    {
        options.clear();
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
            SetLevel("level_1");
            gameLayer->RemoveNode(this);
            break;
        }

        case GameOption::NEW_GAME:
            break;
        case GameOption::CREDITS:
            break;
        }
    }

    void SetLevel(std::string level)
    {
        Map *map = gameLayer->GetNode<Map>();
        map->SetActiveLevel(level);
        auto &entities = map->GetAllEntities();

        for (auto &entity : entities)
        {
            Node *node = CreateEntity(entity);

            if (node == nullptr)
                continue;

            node->SetEntityID(entity.iid);
            gameLayer->AddNode(node);
            node->OnCreate();
        }
    }

    void OnProcess(float fElapsedTime) override
    {
        GetEngine()->Clear(olc::VERY_DARK_BLUE);
        GetEngine()->DrawString(10, 10, "Quest for True Color", olc::WHITE, 2);
        int y = 50;

        if (GetEngine()->GetKey(olc::Key::UP).bPressed)
        {
            selectedOption = static_cast<GameOption>((static_cast<int>(selectedOption) - 1) % options.size());
        }
        else if (GetEngine()->GetKey(olc::Key::DOWN).bPressed)
        {
            selectedOption = static_cast<GameOption>((static_cast<int>(selectedOption) + 1) % options.size());
        }
        else if (GetEngine()->GetKey(olc::Key::ENTER).bPressed)
        {
            OnOptionSelected(selectedOption);
        }

        for (auto option : options)
        {
            olc::Pixel color = selectedOption == option.first ? olc::WHITE : unselectedColor;
            GetEngine()->DrawString(10, y, option.second, color, 2);
            y += 20;
        }
    }
};