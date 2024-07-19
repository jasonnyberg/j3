#include <cstddef>
#include <cstdint>
#include <vector>
#include <bitset>
#include <iostream>

const size_t SLAB_SIZE = 1 << 8;
const size_t NUM_SLABS = 1 << 8;

template <typename T> class CompressedPointer;

template <typename T> class Allocator;

template <typename T> class CompressedPointer {
  private:
    uint16_t slabIndex;
    uint16_t slabOffset;

    static inline Allocator<T> &getAllocator() {
        static Allocator<T> allocator;
        return allocator;
    }

    inline T *getPtr() const { return getAllocator().getPtr(slabIndex, slabOffset); }

  public:
    CompressedPointer() : slabIndex(0), slabOffset(0) {}

    CompressedPointer(uint16_t index, uint16_t offset) : slabIndex(index), slabOffset(offset) {}

    inline operator T *() const { return getPtr(); }
    inline CompressedPointer(T *ptr) { *this = fromRawPtr(ptr); }

    inline T &operator*() const { return *(T *)*this; }
    inline T *operator->() const { return (T *)*this; }

    inline      operator bool() const { return !(slabIndex == 0 && slabOffset == 0); }
    inline bool valid() const { return getAllocator().valid(slabIndex, slabOffset); }

    inline static CompressedPointer<T> allocate() {
        std::pair<uint16_t, uint16_t> result = getAllocator().allocate();
        return CompressedPointer<T>(result.first, result.second);
    }

    inline void deallocate() {
        getAllocator().deallocate(slabIndex, slabOffset);
        slabIndex = slabOffset = 0;
    }

    inline static CompressedPointer<T> fromRawPtr(T *ptr) {
        std::pair<uint16_t, uint16_t> slabInfo = getAllocator().getSlabInfo(ptr);
        return CompressedPointer<T>(slabInfo.first, slabInfo.second);
    }
};

template <typename T> class Allocator {
  private:
    struct Slab {
        T                *items;
        std::vector<bool> inUse;
        size_t            count;

        Slab() : items(new T[SLAB_SIZE]), inUse(SLAB_SIZE, false), count(0) {}

        ~Slab() { delete[] items; }
    };

    Slab                               *slabs[NUM_SLABS] = {nullptr};
    const std::pair<uint16_t, uint16_t> null;

  public:
    Allocator() : null(allocate()) { std::cout << "constructed allocator" << std::endl; }

    ~Allocator() {
        std::cout << "destructed allocator" << std::endl;
        for (size_t i = 0; i < NUM_SLABS; ++i) {
            if (slabs[i]) { delete slabs[i]; }
        }
    }

    std::pair<uint16_t, uint16_t> allocate() {
        for (size_t i = 0; i < NUM_SLABS; ++i) {
            if (slabs[i] && slabs[i]->count == SLAB_SIZE) { continue; }
            if (slabs[i]) {
                size_t j = SLAB_SIZE;
                for (j = 0; j < slabs[i]->inUse.size(); ++j) {
                    if (!slabs[i]->inUse[j]) { break; }
                }
                if (j != SLAB_SIZE) {
                    slabs[i]->inUse[j] = true;
                    ++slabs[i]->count;
                    return std::make_pair(static_cast<uint16_t>(i), static_cast<uint16_t>(j));
                }
            }
        }

        for (size_t i = 0; i < NUM_SLABS; ++i) {
            if (!slabs[i]) {
                slabs[i]           = new Slab();
                slabs[i]->inUse[0] = true;
                ++slabs[i]->count;
                return std::make_pair(static_cast<uint16_t>(i), 0);
            }
        }

        // No more slabs available
        return std::make_pair(0, 0);
    }

    void deallocate(uint16_t slabIndex, uint16_t slabOffset) {
        if (slabIndex == 0 && slabOffset == 0) { return; }

        // Reset the inUse flag and decrement the count
        Slab *slab              = slabs[slabIndex];
        slab->inUse[slabOffset] = false;
        --slab->count;

        if (slab->count == 0) {
            delete slab;
            slabs[slabIndex] = nullptr;
        }
    }

    T *getPtr(uint16_t slabIndex, uint16_t slabOffset) const {
        if (slabIndex == 0 && slabOffset == 0) { return nullptr; }
        Slab *slab = slabs[slabIndex];
        return &slab->items[slabOffset];
    }

    std::pair<uint16_t, uint16_t> getSlabInfo(T *ptr) const {
        for (size_t i = 0; i < NUM_SLABS; ++i) {
            if (slabs[i]) {
                T *slabStart = slabs[i]->items;
                T *slabEnd   = slabStart + SLAB_SIZE;

                if (ptr >= slabStart && ptr < slabEnd) {
                    size_t slabOffset = ptr - slabStart;
                    return std::make_pair(static_cast<uint16_t>(i), static_cast<uint16_t>(slabOffset));
                }
            }
        }

        // Pointer not found in any slab
        return std::make_pair(0, 0);
    }

    bool valid(uint16_t slabIndex, uint16_t slabOffset) const {
        bool valid = false;
        if (slabIndex != 0 || slabOffset != 0) {
            Slab *slab = slabs[slabIndex];
            valid      = slab && slab->inUse[slabOffset];
        }
        std::cout << "valid: " << slabIndex << ", " << slabOffset << (valid ? " true" : " false") << std::endl;
        return valid;
    }
};
