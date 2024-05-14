#ifndef __MEMORY_MANAGER__
#define __MEMORY_MANAGER__

#include <iostream>
#include <vector>

template<typename T>
class MemoryPool 
{
public:
    MemoryPool() : head(nullptr), freePtr(nullptr), remainingSize(0) 
    {
        dealocPtr.reserve(CHUNK_SIZE);
    }

    ~MemoryPool() 
    {
        while (head) 
        {
            MemoryChunk* next = head->nextChunk;
            delete head;
            head = next;
        }
        dealocPtr.clear();
    }

    void* allocate(size_t size) 
    {
        if (!freePtr || remainingSize < size) 
        {
            MemoryChunk* newChunk = new MemoryChunk;
            newChunk->nextChunk = head;
            head = newChunk;
            freePtr = head->data;
            remainingSize = CHUNK_SIZE;
        }
        void* ptr = freePtr;
        freePtr += size;
        remainingSize -= size;
        return ptr;
    }

    template<typename... Args>
    T* newObject(Args&&... args) 
    {
        if (!dealocPtr.empty())
        {
            T* recycledObj = reinterpret_cast<T*>(dealocPtr.back());
            dealocPtr.pop_back();
            memset(recycledObj, 0, sizeof(T));
            new (recycledObj) T(std::forward<Args>(args)...);
            return recycledObj;
        }
        else
        {
            return new (allocate(sizeof(T))) T(std::forward<Args>(args)...);
        }
    }

    template<typename Derived, typename... Args>
    Derived* newDerivedObject(Args&&... args)
    {
        if (!dealocPtr.empty())
        {
            Derived* recycledObj = reinterpret_cast<Derived*>(dealocPtr.back());
            dealocPtr.pop_back();
            memset(recycledObj, 0, sizeof(Derived));
            new (recycledObj) Derived(std::forward<Args>(args)...);
            return recycledObj;
        }
        else
        {
            return new (allocate(sizeof(Derived))) Derived(std::forward<Args>(args)...);
        }
    }

    
    void deleteObject(T* obj)
    {
        obj->~T();
        dealocPtr.push_back((char*)obj);
    }
private:
    static const int CHUNK_SIZE = 1024 * 1024;

    struct MemoryChunk
    {
        MemoryChunk* nextChunk;
        char data[CHUNK_SIZE];
    };

    MemoryChunk* head;
    char* freePtr;
    size_t remainingSize;
    std::vector<char*> dealocPtr;
};


#endif //!__MEMORY_MANAGER__