
using namespace olc::utils::geom2d;

class EntityNode : public CoreNode
{
protected:
    const ldtk::Entity &entity;
    Camera *camera;
    GameImageAssetProvider *spritesProvider;

public:
    EntityNode(const ldtk::Entity &entity, GameNode *game) : CoreNode(entity.iid.str(), game), entity(entity)
    {
        if (game != nullptr)
        {
            camera = game->camera;
            spritesProvider = game->spritesProvider;
        }
    }

    olc::vi2d getSpriteDrawPosition()
    {
        auto &textureRect = entity.getTextureRect();
        return olc::vi2d{textureRect.x, textureRect.y};
    }

    void onCreated() override
    {
        CoreNode::onCreated();

        auto &entityPos = entity.getPosition();
        position = {(float)entityPos.x, (float)entityPos.y};
    }

    void onUpdated(float fElapsedTime) override
    {
        CoreNode::onUpdated(fElapsedTime);
    }
};
