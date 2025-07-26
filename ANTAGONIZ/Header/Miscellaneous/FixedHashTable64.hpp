#ifndef __FIXED_HASH_TABLE_64__
#define __FIXED_HASH_TABLE_64__

#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <cassert>

class FixedHashTable64
{
private:
    uint64_t* table;
    uint32_t* indices;
    size_t capacity;
    uint32_t currentIndex = 0;
    static constexpr uint64_t EMPTY_KEY = 0xFFFFFFFFFFFFFFFFULL;

public:
    explicit FixedHashTable64(size_t size = 1 << 21) : capacity(next_power_of_two(size))  // Always power of two for fast mod
    {
        table = static_cast<uint64_t*>(std::malloc(sizeof(uint64_t) * capacity));
        indices = static_cast<uint32_t*>(std::malloc(sizeof(uint32_t) * capacity));
        assert(table && indices && "Memory allocation failed");

        std::memset(table, 0xFF, sizeof(uint64_t) * capacity); // Fill with EMPTY_KEY
        //std::memset(indices, 0, sizeof(uint32_t) * capacity);
    }

    ~FixedHashTable64()
    {
        std::free(table);
        std::free(indices);
        table = nullptr;
        indices = nullptr;
    }

    void clear()
    {
        std::memset(table, 0xFF, sizeof(uint64_t) * capacity); // Reset all to EMPTY_KEY
        //std::memset(indices, 0, sizeof(uint32_t) * capacity);
        currentIndex = 0;
    }

    bool insert(uint64_t key)
    {
        size_t mask = capacity - 1;
        size_t index = key & mask;

        for (size_t i = 0; i < capacity; ++i)
        {
            size_t probe = (index + i) & mask;
            uint64_t entry = table[probe];

            if (entry == key)
            {
                return false; // already inserted
            }
            else if (entry == EMPTY_KEY)
            {
                table[probe] = key;
                indices[probe] = currentIndex++;
                return true; // inserted
            }
        }

        assert(false && "Hash table overflow - increase capacity");
        return false;
    }

    bool insert_with_index(uint64_t key, uint32_t* out_index)
    {
        size_t mask = capacity - 1;
        size_t index = key & mask;

        for (size_t i = 0; i < capacity; ++i)
        {
            size_t probe = (index + i) & mask;
            uint64_t entry = table[probe];

            if (entry == key)
            {
                *out_index = indices[probe];
                return false; // already present
            }
            else if (entry == EMPTY_KEY)
            {
                table[probe] = key;
                indices[probe] = currentIndex;
                *out_index = currentIndex;
                currentIndex++;
                return true; // new insertion
            }
        }

        assert(false && "Hash table overflow - increase capacity");
        return false;
    }

    bool contains(uint64_t key) const
    {
        size_t mask = capacity - 1;
        size_t index = key & mask;

        for (size_t i = 0; i < capacity; ++i)
        {
            size_t probe = (index + i) & mask;
            uint64_t entry = table[probe];

            if (entry == key) { return true; }
            if (entry == EMPTY_KEY) { return false; }
        }

        return false;
    }

    size_t size_hint() const
    {
        return capacity;
    }

private:
    static size_t next_power_of_two(size_t x)
    {
        size_t power = 1;
        while (power < x) { power <<= 1; }
        return power;
    }
};

#endif //!__FIXED_HASH_TABLE_64__
