#ifndef CHUNK_ARRAY_HPP
#define CHUNK_ARRAY_HPP

#include <cstdlib>   // malloc, realloc, free
#include <cassert>   // assert
#include <utility>   // std::move

template<typename T>
class ChunkArray 
{
    static_assert(std::is_trivial<T>::value, "ChunkArray requires a trivial type T");
public:
    explicit ChunkArray(size_t chunkSize = 8192) : m_capacity(chunkSize), m_size(0)
    {
        m_data = static_cast<T*>(std::malloc(sizeof(T) * m_capacity));
        assert(m_data && "Allocation failed");
    }

    ~ChunkArray() 
    {
        std::free(m_data);
    }

    void push_back(const T& value) 
    {
        if (m_size >= m_capacity)
        {
            grow();
        }
        m_data[m_size++] = value;
    }

    void push_back(T&& value) 
    {
        if (m_size >= m_capacity)
        {
            grow();
        }
        m_data[m_size++] = std::move(value);
    }

    T* push_get_ptr() 
    {
        if (m_size >= m_capacity) 
        {
            grow();
        }
        return &m_data[m_size++];  // incrémente size et renvoie adresse mémoire
    }

    T& operator[](size_t i) 
    {
        assert(i < m_size);
        return m_data[i];
    }

    const T& operator[](size_t i) const 
    {
        assert(i < m_size);
        return m_data[i];
    }

    void clear() 
    {
        m_size = 0;
    }

    void resize(size_t newSize) 
    {
        if (newSize > m_capacity) 
        {
            reserve(newSize);
        }
        m_size = newSize;
    }

    void reserve(size_t newCapacity) 
    {
        if (newCapacity <= m_capacity) 
        {
            return;
        }
        m_capacity = newCapacity;
        T* newData = static_cast<T*>(std::realloc(m_data, sizeof(T) * m_capacity));
        assert(newData && "Reallocation failed");
        m_data = newData;
    }

    size_t size() const { return m_size; }
    size_t capacity() const { return m_capacity; }
    T* data() { return m_data; }
    const T* data() const { return m_data; }

private:
    void grow() 
    {
        m_capacity *= 2;
        T* newData = static_cast<T*>(std::realloc(m_data, sizeof(T) * m_capacity));
        assert(newData && "Reallocation failed");
        m_data = newData;
    }

    T* m_data = nullptr;
    size_t m_capacity = 0;
    size_t m_size = 0;
};

#endif // CHUNK_ARRAY_HPP
