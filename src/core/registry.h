#pragma once

class CoreNodeFactory
{

private:
    std::unordered_map<std::string, std::function<CoreNode *(const ldtk::Entity &, GameNode *)>> registry;

public:
    static CoreNodeFactory &get()
    {
        static CoreNodeFactory instance;
        return instance;
    }

    void registerNode(const std::string &type, std::function<CoreNode *(const ldtk::Entity &, GameNode *)> constructor)
    {
        registry[type] = constructor;
    }

    CoreNode *createNode(const std::string &type, const ldtk::Entity &entity, GameNode *game) const
    {
        auto it = registry.find(type);
        if (it != registry.end())
        {
            return it->second(entity, game);
        }
        return nullptr;
    }
};

template <typename T>
class NodeRegistrar
{
public:
    NodeRegistrar(const std::string &type)
    {
        CoreNodeFactory::get().registerNode(type, [](const ldtk::Entity &entity, GameNode *game) -> CoreNode *
                                        { return new T(entity, game); });
    }
};

#define REGISTER_NODE_TYPE(NODE_TYPE, STRING_ID) \
    static NodeRegistrar<NODE_TYPE> reg_##NODE_TYPE(STRING_ID);