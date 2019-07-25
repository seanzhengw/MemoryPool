#ifndef MEMORYPOOL_H_
#define MEMORYPOOL_H_

#include <stdint.h>
#include <stddef.h>

class MemoryPool
{
public:
    // Create MemoryPool with pre-allocated space pointer, and the space size in bytes.
    MemoryPool(void* p, size_t size);
    // Allocate a n bytes size space.
    void* Alloc(size_t n);
    // Free space p that allocated at this pool.
    void  Free(void* p);
    // Check if space p is allocated at this pool.
    bool Has(void *p);
    // Return usable space size in bytes.
    size_t Usable();

private:
    // pre-allocated space pointer, also the first block info pointer
    void* mBase;
    // size of the pre-allocated space
    size_t mSize;
};

#endif /* _MEMORYPOOL_H */
