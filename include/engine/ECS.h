#pragma once
#include <cstdint>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include <bitset>
#include <memory>
#include <typeindex>
#include <functional>
#include <algorithm>
#include <cassert>

namespace Engine {

using Entity = uint32_t;
constexpr Entity NULL_ENTITY = 0;
constexpr size_t MAX_COMPONENTS = 64;
using ComponentMask = std::bitset<MAX_COMPONENTS>;

// Component type ID generation
inline size_t nextComponentID() {
    static size_t id = 0;
    return id++;
}

template <typename T>
size_t getComponentID() {
    static size_t id = nextComponentID();
    return id;
}

// Type-erased component pool
class IComponentPool {
public:
    virtual ~IComponentPool() = default;
    virtual void remove(Entity e) = 0;
    virtual bool has(Entity e) const = 0;
};

template <typename T>
class ComponentPool : public IComponentPool {
public:
    T& add(Entity e, const T& component) {
        assert(!has(e));
        size_t idx = data_.size();
        data_.push_back(component);
        entityToIndex_[e] = idx;
        indexToEntity_[idx] = e;
        return data_[idx];
    }

    void remove(Entity e) override {
        if (!has(e)) return;
        size_t removedIdx = entityToIndex_[e];
        size_t lastIdx = data_.size() - 1;

        if (removedIdx != lastIdx) {
            data_[removedIdx] = std::move(data_[lastIdx]);
            Entity lastEntity = indexToEntity_[lastIdx];
            entityToIndex_[lastEntity] = removedIdx;
            indexToEntity_[removedIdx] = lastEntity;
        }

        data_.pop_back();
        entityToIndex_.erase(e);
        indexToEntity_.erase(lastIdx);
    }

    T& get(Entity e) {
        return data_[entityToIndex_.at(e)];
    }

    const T& get(Entity e) const {
        return data_[entityToIndex_.at(e)];
    }

    bool has(Entity e) const override {
        return entityToIndex_.find(e) != entityToIndex_.end();
    }

    std::vector<T>& getData() { return data_; }
    const std::unordered_map<Entity, size_t>& getEntityMap() const { return entityToIndex_; }

private:
    std::vector<T> data_;
    std::unordered_map<Entity, size_t> entityToIndex_;
    std::unordered_map<size_t, Entity> indexToEntity_;
};

class World {
public:
    World() : nextEntity_(1) {}

    Entity createEntity() {
        Entity e;
        if (!recycled_.empty()) {
            e = recycled_.back();
            recycled_.pop_back();
        } else {
            e = nextEntity_++;
        }
        entities_.insert(e);
        if (e >= masks_.size()) masks_.resize(e + 1);
        masks_[e].reset();
        return e;
    }

    void destroyEntity(Entity e) {
        for (auto& [id, pool] : pools_) {
            pool->remove(e);
        }
        masks_[e].reset();
        entities_.erase(e);
        recycled_.push_back(e);
    }

    bool isAlive(Entity e) const {
        return entities_.count(e) > 0;
    }

    template <typename T>
    T& addComponent(Entity e, const T& comp = T{}) {
        size_t id = getComponentID<T>();
        ensurePool<T>(id);
        masks_[e].set(id);
        return static_cast<ComponentPool<T>*>(pools_[id].get())->add(e, comp);
    }

    template <typename T>
    void removeComponent(Entity e) {
        size_t id = getComponentID<T>();
        if (pools_.count(id)) {
            static_cast<ComponentPool<T>*>(pools_[id].get())->remove(e);
            masks_[e].reset(id);
        }
    }

    template <typename T>
    T& getComponent(Entity e) {
        size_t id = getComponentID<T>();
        return static_cast<ComponentPool<T>*>(pools_[id].get())->get(e);
    }

    template <typename T>
    const T& getComponent(Entity e) const {
        size_t id = getComponentID<T>();
        return static_cast<const ComponentPool<T>*>(pools_.at(id).get())->get(e);
    }

    template <typename T>
    bool hasComponent(Entity e) const {
        size_t id = getComponentID<T>();
        auto it = pools_.find(id);
        if (it == pools_.end()) return false;
        return it->second->has(e);
    }

    template <typename... Ts>
    std::vector<Entity> view() {
        std::vector<Entity> result;
        ComponentMask required;
        (required.set(getComponentID<Ts>()), ...);

        for (Entity e : entities_) {
            if ((masks_[e] & required) == required) {
                result.push_back(e);
            }
        }
        return result;
    }

    template <typename... Ts, typename Func>
    void each(Func&& func) {
        for (Entity e : view<Ts...>()) {
            func(e, getComponent<Ts>(e)...);
        }
    }

    const std::unordered_set<Entity>& getAllEntities() const { return entities_; }

    void clear() {
        entities_.clear();
        masks_.clear();
        pools_.clear();
        recycled_.clear();
        nextEntity_ = 1;
    }

private:
    template <typename T>
    void ensurePool(size_t id) {
        if (pools_.find(id) == pools_.end()) {
            pools_[id] = std::make_unique<ComponentPool<T>>();
        }
    }

    Entity nextEntity_;
    std::unordered_set<Entity> entities_;
    std::vector<ComponentMask> masks_;
    std::unordered_map<size_t, std::unique_ptr<IComponentPool>> pools_;
    std::vector<Entity> recycled_;
};

} // namespace Engine
