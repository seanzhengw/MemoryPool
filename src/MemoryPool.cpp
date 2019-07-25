#include "MemoryPool.h"

namespace _MemoryPool
{
// Block header
struct PoolBlockInfo
{
    // Is the block in use.
    uint32_t allocated : 1;
    // block size (bytes, include block header)
    uint32_t size : 31;
};
} // namespace _MemoryPool

using _MemoryPool::PoolBlockInfo;

MemoryPool::MemoryPool(void *p, size_t size) : mBase(p), mSize(size)
{
    PoolBlockInfo *first_block = (PoolBlockInfo *)mBase;
    first_block->allocated = 0;
    first_block->size = mSize;
}

void *MemoryPool::Alloc(size_t nbytes)
{
    // let nbytes is a multiple of 4 (32 bits)
    nbytes += (nbytes & 3) ? 4 - (nbytes & 3) : 0;
    size_t blocksize = nbytes + sizeof(PoolBlockInfo);

    // start looking for available block
    PoolBlockInfo *p;
    const PoolBlockInfo *poolend = (PoolBlockInfo *)((uint8_t *)mBase + mSize);
    for (p = (PoolBlockInfo *)mBase; p < poolend; p = (PoolBlockInfo *)((uint8_t *)p + p->size))
    {
        // check block is not allocated and size enough
        if (p->allocated == 1 || p->size < blocksize)
        {
            continue;
        }

        // found a block is available
        p->allocated = 1;

        // check the block has more space than blocksize
        if (p->size > blocksize)
        {
            // setup a block between this block and next block
            PoolBlockInfo *nextblock = (PoolBlockInfo *)((uint8_t *)p + blocksize);
            nextblock->allocated = 0;
            nextblock->size = p->size - blocksize;
        }

        // setup this block
        p->size = blocksize;

        // return addr
        return p + 1;
    }

    // there is no space for nbytes
    return nullptr;
}

void MemoryPool::Free(void *p)
{
    PoolBlockInfo *block = (PoolBlockInfo *)p - 1;
    // check block is in this pool
    if (block < mBase || block > (PoolBlockInfo *)((uint8_t *)mBase + mSize))
    {
        return;
    }

    // check if next block can combine with this block
    PoolBlockInfo *next = (PoolBlockInfo *)((uint8_t *)block + block->size);
    // if this block is at the end of pool, no need to combine next block
    if (next != (PoolBlockInfo *)((uint8_t *)mBase + mSize))
    {
        // combine next block if it's not in use
        if (next->allocated == 0)
        {
            block->size += next->size;
        }
    }

    // mark this block not allocated
    block->allocated = 0;

    // check if previous block can combine with this block
    // start looking for previous block
    PoolBlockInfo *prev = (PoolBlockInfo *)mBase;
    for (; prev < block;)
    {
        PoolBlockInfo *prevnext = (PoolBlockInfo *)((uint8_t *)prev + prev->size);
        // check the next block of prev is this block
        if (prevnext != block)
        {
            prev = prevnext;
            continue;
        }

        // found previous block, combine with this block if it's not in use
        if (prev->allocated == 0)
        {
            prev->size += block->size;
        }
        return;
    }
}

size_t MemoryPool::Usable()
{
    // visit all PoolBlockInfo and count free bytes
    size_t sum = 0;
    PoolBlockInfo *p;
    const PoolBlockInfo *poolend = (PoolBlockInfo *)((uint8_t *)mBase + mSize);
    for (p = (PoolBlockInfo *)mBase; p < poolend; p = (PoolBlockInfo *)((uint8_t *)p + p->size))
    {
        if (p->allocated == 0)
        {
            sum += p->size - sizeof(PoolBlockInfo);
        }
    }
    return sum;
}

bool MemoryPool::Has(void *p)
{
    return (p >= mBase + 1) && (p < (PoolBlockInfo *)((uint8_t *)mBase + mSize));
}