class ShellGame : public MiniGame
{
private:
    const uint8_t shellCount = 3;
    bool didMove = false;
    bool didDisplayShell = false;
    float moveTime = 0.0f;

public:
    ShellGame(GameNode *game) : MiniGame("ShellGame", game)
    {
    }

    void onCreated() override
    {
        MiniGame::onCreated();

        game->addDialog({"Pick the right shell by pressing 1,2 or 3", 2.0f});
        didDisplayShell = false;

        // Randomly select a shell to put the gem under
        int gemWillBeUnder = rand() % shellCount;

        const auto spriteHalfSize = SPRITE_SIZE * 0.5f;
        const auto spritePositionY = SCREEN_HEIGHT * 0.5f - spriteHalfSize;
        const auto screenHalfWidth = SCREEN_WIDTH * 0.5f;
        const auto padding = 20;

        const auto totalWidth = (shellCount * SPRITE_SIZE) + ((shellCount - 1) * padding);

        for (int i = 0; i < shellCount; i++)
        {
            auto *shell = new ShellNode(game);
            shell->onCreated();

            if (i == gemWillBeUnder)
            {
                auto *gem = new GemNode(game);
                gem->onCreated();
                gem->parent = shell;
                shell->addChild(gem);
            }

            auto position = shell->position;
            position.y = spritePositionY;
            position.x = screenHalfWidth - totalWidth * 0.5f + (i * (SPRITE_SIZE + padding));
            shell->setPosition(position);

            shell->name = "shell_" + std::to_string(i + 1);
            shell->display();

            addChild(shell);
        }
    }

    void onUpdated(float fElapsedTime) override
    {
        MiniGame::onUpdated(fElapsedTime);

        if (moveTime < 4)
        {
            moveTime += fElapsedTime;
            scrambleShells();
        }
        else if (!game->getFlag("DidTeachHowToPlayShellGame"))
        {
            game->addDialog({"Press 1, 2 or 3 to select a shell", 3.0f});
            game->setFlag("DidTeachHowToPlayShellGame", true);
        }
        else
        {
            evaluateShells();
        }
    }

    void displayShell()
    {
        if (didDisplayShell)
        {
            return;
        }

        for (int i = 0; i < shellCount; i++)
        {
            auto *shell = getChildOfType<ShellNode>("shell_" + std::to_string(i + 1));
            if (!shell)
            {
                return;
            }

            shell->display();
        }
    }

    void evaluateShells()
    {
        for (int i = 0; i < shellCount; i++)
        {
            auto *shell = getChildOfType<ShellNode>("shell_" + std::to_string(i + 1));
            if (!shell)
            {
                return;
            }

            auto key = olc::Key::K1 + i;
            if (Pressed(key))
            {
                finishGame(isTheRightShell(i + 1));
                return;
            }
        }
    }

    bool isTheRightShell(int shellIndex)
    {
        auto *shell = getChildOfType<ShellNode>("shell_" + std::to_string(shellIndex));
        if (!shell)
        {
            return false;
        }

        auto *gem = shell->getChildOfType<GemNode>();
        return gem != nullptr;
    }

    void switchShellPosition(ShellNode *shell1, ShellNode *shell2)
    {
        if (!shell1 || !shell2)
        {
            return;
        }

        if (shell1->position == shell2->position)
        {
            return;
        }

        if (shell1->getIsMoving() || shell2->getIsMoving())
        {
            return;
        }

        auto position1 = shell1->position;
        auto position2 = shell2->position;

        shell1->moveTo(position2);
        shell2->moveTo(position1);
    }

    void scrambleShells()
    {
        auto *shell1 = getChildOfType<ShellNode>("shell_1");
        auto *shell2 = getChildOfType<ShellNode>("shell_2");
        auto *shell3 = getChildOfType<ShellNode>("shell_3");

        if (!shell1 || !shell2 || !shell3)
        {
            return;
        }

        if (shell1->getIsMoving() || shell2->getIsMoving() || shell3->getIsMoving())
        {
            return;
        }

        std::random_device r;
        std::seed_seq seed{r(), r(), r(), r(), r(), r(), r(), r()};
        std::mt19937 eng(seed);

        auto shells = std::vector<ShellNode *>{shell1, shell2, shell3};
        std::shuffle(shells.begin(), shells.end(), eng);

        for (int i = 0; i < shellCount; i++)
        {
            auto *shell = shells[i];
            auto *nextShell = shells[(i + 1) % shellCount];
            switchShellPosition(shell, nextShell);
        }
    }
};