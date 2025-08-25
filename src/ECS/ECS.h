#pragma once

#include "../Logger.h"
#include <bitset>
#include <deque>
#include <memory>
#include <set>
#include <typeindex>
#include <unordered_map>
#include <utility>
#include <vector>

const unsigned int MAX_COMPONENTS = 32;

typedef std::bitset<MAX_COMPONENTS> Signature;

struct IComponent {
protected:
    static int nextId;
};

template <typename T> class Component : public IComponent {
public:
    static int GetId() {
        static auto id = nextId++;
        return id;
    }
};

class Entity {
private:
    int id;

public:
    Entity(int id)
    : id(id) {};
    Entity(const Entity &entity) = default;
    int GetId() const { return id; };
    Entity &operator=(const Entity &other) = default;
    bool operator==(const Entity &other) const { return id == other.id; }
    bool operator!=(const Entity &other) const { return id != other.id; }
    bool operator<(const Entity &other) const { return id < other.id; }
    bool operator>(const Entity &other) const { return id > other.id; }

    template <typename TComponent, typename... TArgs>
    void AddComponent(TArgs &&...args);
    template <typename TComponent> void RemoveComponent();
    template <typename TComponent> bool HasComponent() const;
    template <typename TComponent> TComponent &GetComponent() const;
    void Kill();

    void Tag(const std::string &tag);
    bool HasTag(const std::string &tag) const;
    void Group(const std::string &group);
    bool BelongsToGroup(const std::string &group) const;

    class Registry *registry;
};

class System {
private:
    Signature componentSignature;
    std::vector<Entity> entities;

public:
    System() = default;
    ~System() = default;
    void AddEntityToSystem(Entity entity);
    void RemoveEntityFromSystem(Entity entity);
    const std::vector<Entity> &GetSystemEntities() const;
    const Signature &GetComponentSignature() const;
    template <typename TComponent> void RequireComponent();
};

class IPool {
public:
    virtual ~IPool() = default;
    virtual void RemoveEntity(int entityId) = 0;
};

template <typename T> class Pool : public IPool {
private:
    std::vector<T> data;
    unsigned int size;
    std::unordered_map<int, int> entityIdToIndex;
    std::unordered_map<int, int> indexToEntityId;

public:
    Pool(int capacity = 100) {
        size = 0;
        data.resize(capacity);
    }
    virtual ~Pool() = default;

    bool isEmpty() { return size == 0; }

    unsigned int GetSize() { return size; }

    void Clear() {
        data.clear();
        size = 0;
    }

    void Add(T object) {
        data.push_back(object);
    }

    void Set(int entityId, T object) {
        if (entityIdToIndex.find(entityId) != entityIdToIndex.end()) {
            int index = entityIdToIndex[entityId];
            data[index] = object;
        } else {
            unsigned int index = size;
            entityIdToIndex.emplace(entityId, index);
            indexToEntityId.emplace(index, entityId);
            if (index >= data.capacity()) {
                data.resize(size * 2);
            }
            data[index] = object;
            size += 1;
        }
    }

    void Remove(int entityId) {
        int indexOfRemoved = entityIdToIndex[entityId];
        int indexOfLast = size - 1;
        data[indexOfRemoved] = data[indexOfLast];

        int entityIdOfLastElement = indexToEntityId[indexOfLast];
        entityIdToIndex[entityIdOfLastElement] = indexOfRemoved;
        indexToEntityId[indexOfRemoved] = entityIdOfLastElement;

        entityIdToIndex.erase(entityId);
        indexToEntityId.erase(indexOfLast);

        size -= 1;
    }

    T& Get(int entityId) {
        int index = entityIdToIndex[entityId];
        return static_cast<T&>(data[index]);
    }

    T& operator [](unsigned int index) {
        return data[index];
    }

    void RemoveEntity(int entityId) override {
        if (entityIdToIndex.find(entityId) == entityIdToIndex.end()) {
            return;
        }
        Remove(entityId);
    }
};

class Registry {
private:
    int numEntities = 0;
    std::set<Entity> entitiesToBeAdded;
    std::set<Entity> entitiesToBeKilled;
    std::deque<int> freeIds;

    // Vector index = component type ID
    // Pool index = entity ID
    std::vector<std::shared_ptr<IPool>> componentPools;

    // Vector index = entity ID
    std::vector<Signature> entityComponentSignatures;

    std::unordered_map<std::type_index, std::shared_ptr<System>> systems;

    std::unordered_map<std::string, Entity> entityPerTag;
    std::unordered_map<int, std::string> tagPerEntity;
    std::unordered_map<std::string, std::set<Entity>> entitiesPerGroup;
    std::unordered_map<int, std::string> groupPerEntity;

public:
    Registry() = default;
    void Update();

    Entity CreateEntity();
    void KillEntity(Entity entity);

    template <typename TComponent, typename... TArgs>
    void AddComponent(Entity entity, TArgs &&...args);
    template <typename TComponent> void RemoveComponent(Entity entity);
    template <typename TComponent> bool HasComponent(Entity entity) const;
    template <typename TComponent>
    TComponent &GetComponent(Entity entity) const;

    template <typename TSystem, typename... TArgs>
    void AddSystem(TArgs &&...args);
    template <typename TSystem> void RemoveSystem();
    template <typename TSystem> bool HasSystem() const;
    template <typename TSystem> TSystem &GetSystem() const;
    void AddEntityToSystems(Entity entity);
    void RemoveEntityFromSystems(Entity entity);

    void TagEntity(Entity entity, const std::string &tag);
    bool EntityHasTag(Entity entity, const std::string &tag) const;
    Entity GetEntityByTag(const std::string &tag) const;
    void RemoveEntityTag(Entity entity);

    void GroupEntity(Entity entity, const std::string &group);
    bool EntityBelongsToGroup(Entity entity, const std::string &group) const;
    std::vector<Entity> GetEntitiesByGroup(const std::string &group) const;
    void RemoveEntityGroup(Entity entity);
};

template <typename TComponent> void System::RequireComponent() {
    const auto componentId = Component<TComponent>::GetId();
    componentSignature.set(componentId);
}

template <typename TComponent, typename... TArgs>
void Entity::AddComponent(TArgs &&...args) {
    registry->AddComponent<TComponent>(*this, std::forward<TArgs>(args)...);
}

template <typename TComponent> void Entity::RemoveComponent() {
    registry->RemoveComponent<TComponent>(*this);
}

template <typename TComponent> bool Entity::HasComponent() const {
    return registry->HasComponent<TComponent>(*this);
}

template <typename TComponent> TComponent &Entity::GetComponent() const {
    return registry->GetComponent<TComponent>(*this);
}

template <typename TComponent, typename... TArgs>
void Registry::AddComponent(Entity entity, TArgs &&...args) {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    if (componentId >= static_cast<int>(componentPools.size())) {
        componentPools.resize(componentId + 1, nullptr);
    }
    if (!componentPools[componentId]) {
        std::shared_ptr<Pool<TComponent>> newComponentPool =
            std::make_shared<Pool<TComponent>>();
        componentPools[componentId] = newComponentPool;
    }
    std::shared_ptr<Pool<TComponent>> componentPool =
        std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);

    TComponent newComponent(std::forward<TArgs>(args)...);
    componentPool->Set(entityId, newComponent);
    entityComponentSignatures[entityId].set(componentId);

    Logger::Log(
        "Component id = " + std::to_string(componentId) +
        " was added to entity id " + std::to_string(entityId)
    );
}

template <typename TComponent> void Registry::RemoveComponent(Entity entity) {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();

    std::shared_ptr<Pool<TComponent>> componentPool =
        std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
    componentPool->Remove(entityId);

    entityComponentSignatures[entityId].set(componentId, false);
}

template <typename TComponent>
bool Registry::HasComponent(Entity entity) const {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();
    return entityComponentSignatures[entityId].test(componentId);
}

template <typename TComponent>
TComponent &Registry::GetComponent(Entity entity) const {
    const auto componentId = Component<TComponent>::GetId();
    const auto entityId = entity.GetId();
    const auto componentPool =
        std::static_pointer_cast<Pool<TComponent>>(componentPools[componentId]);
    return componentPool->Get(entityId);
}

template <typename TSystem, typename... TArgs>
void Registry::AddSystem(TArgs &&...args) {
    std::shared_ptr<TSystem> newSystem =
        std::make_shared<TSystem>(std::forward<TArgs>(args)...);
    systems.insert(std::make_pair(std::type_index(typeid(TSystem)), newSystem));
}

template <typename TSystem> void Registry::RemoveSystem() {
    auto system = systems.find(std::type_index(typeid(TSystem)));
    systems.erase(system);
}

template <typename TSystem> bool Registry::HasSystem() const {
    return systems.find(std::type_index(typeid(TSystem))) != systems.end();
}

template <typename TSystem> TSystem &Registry::GetSystem() const {
    const auto pair = systems.find(std::type_index(typeid(TSystem)));
    return *(std::static_pointer_cast<TSystem>(pair->second));
}
