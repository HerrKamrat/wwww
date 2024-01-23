#pragma once

#include <algorithm>
#include <stddef.h>

template <typename T, size_t size = 100>
struct ObjectPool {
    using Handle = uint32_t;
    const static Handle invalid_handle = 0;

    struct Slot {
        uint16_t check;
        T object;
    };

    struct Iterator {
        Iterator(Slot* ptr, Slot* begin, Slot* end) : ptr(ptr), begin(begin), end(end) {
        }

        Slot *ptr, *begin, *end;
        T& operator*() const {
            return ptr->object;
        }
        bool operator!=(const Iterator& b) const {
            return ptr != b.ptr;
        }

        Iterator& operator++() {
            do {
                ptr++;
            } while (ptr < end && ((ptr->check & 0x1) == 0));
            return *this;
        }
    };

    Slot objects[size];

    Iterator begin() {
        auto begin = std::begin(objects);
        auto end = std::end(objects);
        Iterator it{begin - 1, begin, end};
        return ++it;
    }
    Iterator end() {
        auto begin = std::begin(objects);
        auto end = std::end(objects);
        return {end, begin, end};
    }

    Handle alloc() {
        auto begin = std::begin(objects);
        auto end = std::end(objects);
        auto slot = std::find_if(std::begin(objects), std::end(objects),
                                 [](const Slot& slot) { return (slot.check & 0x1) == 0; });
        if (slot == std::end(objects)) {
            trace("no available object in pool");
            return invalid_handle;
        }

        uint16_t check = slot->check = slot->check + 1;
        uint16_t index = (uint16_t)std::distance(begin, slot);

        return (Handle)(check << 16 | index);
    };

    template <typename... Args>
    Handle create(Args&&... args) {
        auto handle = alloc();
        if (handle) {
            auto ptr = get(handle);
            T t{std::forward<Args>(args)...};
            *ptr = T{std::forward<Args>(args)...};
        }
        return handle;
    };

    Slot* getSlot(Handle handle) {
        uint16_t index = (uint16_t)handle;
        uint16_t check = (uint16_t)(handle >> 16);

        if ((check & 0x1) == 0) {
            return nullptr;
        }

        Slot& slot = objects[index];
        if (slot.check != check) {
            return nullptr;
        }

        return &slot;
    }

    void free(T* obj) {
        const char* ptr = reinterpret_cast<const char*>(obj);
        const char* first = reinterpret_cast<const char*>(&std::begin(objects)->object);

        if (ptr < first) {
            return;
        }

        auto s = sizeof(Slot);
        auto d = static_cast<decltype(s)>(std::distance(first, ptr));
        auto i = d / s;

        if (i >= 0 && i < size && (objects[i].check & 0x1) != 0) {
            objects[i].check += 1;
        }
    };

    void free(Handle handle) {
        auto slot = getSlot(handle);
        if (slot) {
            slot->check += 1;
        }
    };

    T* get(Handle handle) {
        auto slot = getSlot(handle);
        if (slot) {
            return &(slot->object);
        }
        tracef("get: %d, no slot", handle);
        return nullptr;
    };
};
