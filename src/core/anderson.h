
using namespace olc::utils::geom2d;

class Anderson : public Item
{
private:
    AnimationController *idle;
    AnimationController *idleVillain;
    float delta = 0.0f;
    olc::vf2d *position;
    bool dialogMode = false;
    bool didSmell = false;
    bool isVillain = false;
    bool walkAway = false;

public:
    void OnEntityDefined(const ldtk::Entity &entity) override
    {
        Item::OnEntityDefined(entity);
        auto initialPosition = entity.getWorldPosition();
        position = new olc::vf2d({static_cast<float>(initialPosition.x), static_cast<float>(initialPosition.y)});
        idle = new AnimationController(this, 0.4f, 0, {0, 1});
        idleVillain = new AnimationController(this, 0.4f, 0, {2, 2});
    }

    AnimationController *GetCurrentAnimation()
    {
        if (isVillain)
            return idleVillain;
        return idle;
    }

    void OnScreen(float fElapsedTime) override
    {
        Map *map = GetLayer()->GetNode<Map>();
        Player *player = GetLayer()->GetNode<Player>();

        if (player == nullptr)
            return;

        dialogMode = overlaps(*player->GetCollider(), GetCollider()) && player->HasFlower();

        if (delta < 0.5f)
        {
            delta += fElapsedTime;
        }

        if (didSmell && !map->HasDialogs() && !isVillain)
        {
            player->EnableControls();
            dialogMode = false;
            map->AddDialog("Anderson: Whoa! I don't feel too good", true);
            map->AddDialog("Anderson: I think I'm allergic to this flower");
            map->AddDialog("THE CURSED FLOWER TURNED ANDERSON INTO A VILLAIN!");
            isVillain = true;
        }
        else if (isVillain && !map->HasDialogs() && !walkAway)
        {
            map->AddDialog("Anderson: I will cause mayhem! hahaha >:-C", true);
            map->AddDialog("Anderson: FYI I'm not gonna destroy the world");
            map->AddDialog("Anderson: I'm just gonna make it a little bit more annoying");
            walkAway = true;
        }
        else if (walkAway && !map->HasDialogs())
        {
            if (delta < 2.0f)
                delta += fElapsedTime;
            else
            {
                player->EnableControls();
                GetLayer()->RemoveNode(this);
                dialogMode = false;
                Flower *flower = GetLayer()->GetNode<Flower>();
                GetLayer()->RemoveNode(flower);
                return;
            }

            position->y -= delta * 0.8;
        }

        if (dialogMode)
        {
            player->DisableControls();

            if (!didSmell)
            {
                map->AddDialog("Anderson: Hello, I'm Anderson!", true);
                map->AddDialog("Anderson: What a nice blue flower... Can I smell it?");
                map->AddDialog("     You: Actually, it is purple.");
                map->AddDialog("Anderson: ...");
                map->AddDialog("Anderson: bruh >:-C");
                map->AddDialog("Anderson: Anyway, can I smell it?");
                map->AddDialog("     You: Sure, go ahead.");
                didSmell = true;
            }
        }

        auto *animation = GetCurrentAnimation();
        auto frame = animation->GetFrame(fElapsedTime);
        auto pos = *position;
        pos.y += 8;
        map->DrawTile(pos, animation->tilesetId, frame, {32.0f, 32.0f});
    }
};