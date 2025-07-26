#ifndef __LARGE_ARRAY__
#define __LARGE_ARRAY__

#include <iostream>
#include <vector>
#include <memory>
#include <stdexcept>
#include <cstring>

template <typename T>
class LargeArray 
{
private:
    static constexpr size_t MAX_CHUNK_SIZE = 2ULL * 1024 * 1024 * 1024 / sizeof(T);

    std::vector<std::unique_ptr<T[]>> chunks;
    size_t totalSize;

    std::pair<size_t, size_t> getChunkIndex(size_t index) const 
    {
        size_t chunkIndex = index / MAX_CHUNK_SIZE;
        size_t innerIndex = index % MAX_CHUNK_SIZE;
        return { chunkIndex, innerIndex };
    }

public:
    LargeArray(size_t size) : totalSize(size) 
    {
        size_t numChunks = (size + MAX_CHUNK_SIZE - 1) / MAX_CHUNK_SIZE;
        chunks.reserve(numChunks);

        for (size_t i = 0; i < numChunks; ++i) 
        {
            size_t chunkSize = std::min(MAX_CHUNK_SIZE, size - i * MAX_CHUNK_SIZE);
            chunks.emplace_back(std::make_unique<T[]>(chunkSize));
        }
    }

    T& operator[](size_t index) 
    {
        if (index >= totalSize) 
        {
            throw std::out_of_range("Index out of range");
        }
        auto [chunkIndex, innerIndex] = getChunkIndex(index);
        return chunks[chunkIndex][innerIndex];
    }

    const T& operator[](size_t index) const 
    {
        if (index >= totalSize) 
        {
            throw std::out_of_range("Index out of range");
        }
        auto [chunkIndex, innerIndex] = getChunkIndex(index);
        return chunks[chunkIndex][innerIndex];
    }

    size_t size() const 
    {
        return totalSize;
    }
};

#endif //!__LARGE_ARRAY__