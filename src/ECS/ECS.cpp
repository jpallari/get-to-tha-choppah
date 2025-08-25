#include "ECS.h"
#include "../Logger.h"
#include <algorithm>

int IComponent::nextId = 0;

void Entity::Kill() { registry->KillEntity(*this); }

void Entity::Tag(const std::string &tag) { registry->TagEntity(*this, tag); }
bool Entity::HasTag(const std::string &tag) const {
    return registry->EntityHasTag(*this, tag);
}
void Entity::Group(const std::string &group) {
    registry->GroupEntity(*this, group);
}
bool Entity::BelongsToGroup(const std::string &group) const {
    return registry->EntityBelongsToGroup(*this, group);
}

void System::AddEntityToSystem(Entity entity) { entities.push_back(entity); }

void System::RemoveEntityFromSystem(Entity entity) {
    entities.erase(
        std::remove_if(
            entities.begin(),
            entities.end(),
            [&entity](Entity other) { return entity == other; }
        ),
        entities.end()
    );
}

const std::vector<Entity> &System::GetSystemEntities() const {
    return entities;
}

const Signature &System::GetComponentSignature() const {
    return componentSignature;
}

Entity Registry::CreateEntity() {
    int entityId;

    if (freeIds.empty()) {
        entityId = numEntities++;
        if (entityId >= static_cast<int>(entityComponentSignatures.size())) {
            entityComponentSignatures.resize(entityId + 1);
        }
    } else {
        entityId = freeIds.front();
        freeIds.pop_front();
    }

    Entity entity(entityId);
    entity.registry = this;
    entitiesToBeAdded.insert(entity);

    Logger::Log("Entity created with id = " + std::to_string(entityId));
    return entity;
}

void Registry::KillEntity(Entity entity) { entitiesToBeKilled.insert(entity); }

void Registry::AddEntityToSystems(Entity entity) {
    const auto entityId = entity.GetId();
    const auto &entityComponentSignature = entityComponentSignatures[entityId];
    for (auto &system : systems) {
        const auto &systemComponentSignature =
            system.second->GetComponentSignature();
        bool isInterested =
            (entityComponentSignature & systemComponentSignature) ==
            systemComponentSignature;
        if (isInterested) {
            system.second->AddEntityToSystem(entity);
        }
    }
}

void Registry::RemoveEntityFromSystems(Entity entity) {
    const auto entityId = entity.GetId();
    const auto &entityComponentSignature = entityComponentSignatures[entityId];
    for (auto &system : systems) {
        const auto &systemComponentSignature =
            system.second->GetComponentSignature();
        bool isInterested =
            (entityComponentSignature & systemComponentSignature) ==
            systemComponentSignature;
        if (isInterested) {
            system.second->RemoveEntityFromSystem(entity);
        }
    }
}

void Registry::Update() {
    for (auto &entity : entitiesToBeAdded) {
        AddEntityToSystems(entity);
    }
    entitiesToBeAdded.clear();

    for (auto &entity : entitiesToBeKilled) {
        const int id = entity.GetId();
        RemoveEntityFromSystems(entity);
        entityComponentSignatures[id].reset();

        for (auto pool : componentPools) {
            if (pool) {
                pool->RemoveEntity(entity.GetId());
            }
        }

        freeIds.push_back(id);
        RemoveEntityTag(entity);
        RemoveEntityGroup(entity);
    }
    entitiesToBeKilled.clear();
}

void Registry::TagEntity(Entity entity, const std::string &tag) {
    entityPerTag.emplace(tag, entity);
    tagPerEntity.emplace(entity.GetId(), tag);
}
bool Registry::EntityHasTag(Entity entity, const std::string &tag) const {
    if (tagPerEntity.find(entity.GetId()) == tagPerEntity.end()) {
        return false;
    }
    return entityPerTag.find(tag)->second == entity;
}
Entity Registry::GetEntityByTag(const std::string &tag) const {
    return entityPerTag.at(tag);
}
void Registry::RemoveEntityTag(Entity entity) {
    const auto taggedEntity = tagPerEntity.find(entity.GetId());
    if (taggedEntity != tagPerEntity.end()) {
        const auto tag = taggedEntity->second;
        entityPerTag.erase(tag);
        tagPerEntity.erase(taggedEntity);
    }
}

void Registry::GroupEntity(Entity entity, const std::string &group) {
    entitiesPerGroup.emplace(group, std::set<Entity>());
    entitiesPerGroup[group].emplace(entity);
    groupPerEntity.emplace(entity.GetId(), group);
}
bool Registry::EntityBelongsToGroup(
    Entity entity, const std::string &group
) const {
    if (groupPerEntity.find(entity.GetId()) == groupPerEntity.end()) {
        return false;
    }
    return groupPerEntity.find(entity.GetId())->second == group;
}
std::vector<Entity>
Registry::GetEntitiesByGroup(const std::string &group) const {
    const auto &entities = entitiesPerGroup.at(group);
    return std::vector<Entity>(entities.begin(), entities.end());
}
void Registry::RemoveEntityGroup(Entity entity) {
    const auto &groupEntry = groupPerEntity.find(entity.GetId());
    if (groupEntry == groupPerEntity.end()) {
        return;
    }
    const auto group = entitiesPerGroup.find(groupEntry->second);
    if (group != entitiesPerGroup.end()) {
        const auto entityInGroup = group->second.find(entity);
        if (entityInGroup != group->second.end()) {
            group->second.erase(entityInGroup);
        }
    }
    groupPerEntity.erase(groupEntry);
}
