#pragma once
#include <iostream>
#include <typeinfo>

template <typename T>
struct DefaultRAIIDeleter {
    static void deleter(T& t) {
        t.release();
    }
};

template <typename T>
struct RAIIDeleter : DefaultRAIIDeleter<T> {};

template <typename T>
class RAIIWrapper {
public:
    RAIIWrapper() = default;
    RAIIWrapper(T tRaw) : mRaw(tRaw){};

    RAIIWrapper(std::nullptr_t) : mRaw(nullptr){};

    RAIIWrapper& operator=(RAIIWrapper&& tOther) {
        if (mRaw) {
            RAIIDeleter<T>::deleter(mRaw);
        }
        mRaw = tOther.mRaw;
        tOther.mRaw = nullptr;
        return *this;
    }
    RAIIWrapper(RAIIWrapper&& tOther) {
        mRaw = tOther.mRaw;
        tOther.mRaw = nullptr;
    }

    RAIIWrapper& operator=(const RAIIWrapper& tOther) = delete;

    RAIIWrapper(const RAIIWrapper& tOther) = delete;

    auto operator->() {
        if constexpr (std::is_pointer_v<T>) {
            return mRaw;
        } else {
            return &mRaw;
        }
    }

    T& operator*() {
        return mRaw;
    }

    const T& operator*() const {
        return mRaw;
    }

    ~RAIIWrapper() {
        if (mRaw) {
            RAIIDeleter<T>::deleter(mRaw);
        }
    }

    operator T() {
        return mRaw;
    }

private:
    T mRaw = nullptr;
};