#pragma once
#include <cstdint>

#include "entities.hpp"

class Command {
public:
    void fill(Entities* world) {
        mWorld = world;
    }

    template <typename... T>
    Query<T...> query() {
        Query<T...> result{};

        result.fill(mWorld);
        return result;
    }

    template <typename... T>
    void removeComponents(EntityID entity) {
        assert(mWorld);

        mWorld->removeComponents<T...>(entity);
    }

    template <typename... T>
    void setGlobal(T... values) {
        assert(mWorld);
        mWorld->setGlobal<T...>(std::move(values)...);
    }

    template <typename... T>
    auto getGlobal() {
        return mWorld->getGlobal<T...>();
    }

private:
    Entities* mWorld;
};