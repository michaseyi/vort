#pragma once

#include <functional>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>

using EventListener = std::function<void(const void *)>;

class EventEmitter {
public:
    template <typename T>
    void registerListener(std::function<void(const T *)> tListener) {
        auto typeIndex = std::type_index(typeid(T));

        mListeners[typeIndex].emplace_back([tListener](const void *raw) {
            tListener(reinterpret_cast<const T *>(raw));
        });
    }

    template <typename T>
    void emitEvent(T eventData) {
        auto typeIndex = std::type_index(typeid(T));

        auto listenersIter = mListeners.find(typeIndex);

        if (listenersIter == mListeners.end()) {
            return;
        }

        for (auto &listener : listenersIter->second) {
            listener(&eventData);
        }
    }
    // void unregisterListener();

private:
    std::unordered_map<std::type_index, std::vector<EventListener>> mListeners;
};
