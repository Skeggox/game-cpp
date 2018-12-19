#include "memoryallocator.hpp"

#include <cstdio>

#ifdef __GNUC__ 
// The third parameter is temporal locality. Use profiler
// to tune (0 -> 3)
#define CACHE_PRELOAD_READ(x) __builtin_prefetch(x, 0, 2)
#define CACHE_PRELOAD_WRITE(x) __builtin_prefetch(x, 1, 2)
#else
#define CACHE_PRELOAD_READ(x) 
#define CACHE_PRELOAD_WRITE(x)
#endif

#ifdef DEBUG
#define TRACE(...) printf(__VA_ARGS__)
#endif

/*
 * This is (hopefully) a simple, cache-friendly block allocator. 
 *
 * Designed this from first principles for this assignment.
 * 
 * The idea behind this allocator is that a cache optimized linear search
 * O(n) is sometimes faster than a more complex search O(log n) for smaller values of n
 * In this allocator, the per-allocation metadata is stored in cache-friendly bit arrays,
 * not in a header before the allocated memory.
 * This reduces interior fragmention quite a bit for lots of small allocations.
 *
 * Recommended usage:
 * - Allocate transient objects during update loop in game engine on resource restricted platforms, like mobile.
 *
 * NOTE: Allocator must be tuned using profiler!
 */

/*
 * The freeBlocksMask is a bit array, where each bit indicates
 * if it is a free block or not.
 * The finalBlocksMask is a bit array where each bit indicates
 * if it is the final block for that allocation
 * 
 * Given an empty pool of 8 blocks:
 * free =  11111111
 * final = 00000000
 *
 * Allocating 2 blocks:
 * free =  00111111
 * final = 01000000
 *
 * Allocating 3 blocks:
 * free =  00000111
 * final = 01001000
 *
 * Freeing first 2 blocks:
 * free =  11000111
 * final = 00001000
 *
 * Invariants:
 * - The number of bits set in finalBlocksMask == number of allocations
 * - The number of groups of zero bits in freeBlocksMask == number of allocations
 *
 * TODO: Add invariant checks to the unit tests
 * TODO: Confirm the compiler properly optimizes the block bit mask operations
 * - It should properly inline the operations, then unroll some of the loops, etc.
 * TODO: Use the profiler to confirm the preloads are improving performance
 */

inline bool MemoryAllocator::isBlockSet(int index, mask_type masks[])
{
    int shift = index & (sizeof(mask_type) - 1);
    mask_type mask = masks[index / sizeof(mask_type)];
    return (mask >> shift) & 1UL;
}

inline void MemoryAllocator::setBlock(int index, mask_type masks[])
{
    int shift = index & (sizeof(mask_type) - 1);
    mask_type& mask = masks[index / sizeof(mask_type)];
    mask |= 1UL << shift;
}

inline void MemoryAllocator::clearBlock(int index, mask_type masks[])
{
    int shift = index & (sizeof(mask_type) - 1);
    mask_type& mask = masks[index / sizeof(mask_type)];
    mask &= ~(1UL << shift);
}

inline int roundUpToPower2(int value, int multiple)
{
    return (value + multiple - 1) & -multiple;
}

MemoryAllocator::MemoryAllocator()
{
    for(auto& free : freeBlocksMask)
    {
        free = -1;
    }

    for(auto& final : finalBlocksMask)
    {
        final = 0;
    }
}

MemoryAllocator::~MemoryAllocator()
{
}

void* MemoryAllocator::Allocate(const size_t size)
{
    TRACE("Allocating %zu bytes\n", size);

    CACHE_PRELOAD_READ(freeBlocksMask);

    int blocksNeeded = roundUpToPower2(size, sizeof(block_type)) / sizeof(block_type);
 
    TRACE("Blocks Needed: %i\n", blocksNeeded);

    if (blocksNeeded > BlockCount)
    {
        TRACE("Too large\n");
        return nullptr;
    }

    // Look for the first set of free blocks that can hold this allocation
    // Optimize: keep track of first and last free block indexes, to reduce the search range
    int blocksFound = 0;
    int freeIndex = -1;

    for(int index = 0; index < BlockCount; ++index)
    {
        if (isBlockSet(index, freeBlocksMask))
        {
            if (blocksFound == 0)
            {
                TRACE("Checking free area at %i\n", index);
                freeIndex = index;
            }
            blocksFound++;

            if (blocksFound == blocksNeeded)
            {
                break;
            }
        }
        else
        {
            blocksFound = 0;
            freeIndex = -1;
        }
    }

    if (freeIndex > -1)
    {
        CACHE_PRELOAD_WRITE(&freeBlocksMask[freeIndex]);
        CACHE_PRELOAD_WRITE(&finalBlocksMask[freeIndex]);

        // Mark blocks as free, and set the final block
        int endIndex = freeIndex + blocksNeeded;
        
        TRACE("Free area found at %i -> %i\n", freeIndex, endIndex);
        
        for(int i = freeIndex; i < endIndex; ++i)
        {
            clearBlock(i, freeBlocksMask);
        }
        setBlock(endIndex - 1, finalBlocksMask);
        return &memoryPool[freeIndex];
    }
    else
    {
        TRACE("Could not find a free area\n");
        // Could not find a free area - throw bad_alloc or just return null?
        return nullptr;
    }
}

void MemoryAllocator::Free(const void* address)
{
    const block_type *blockAddress = static_cast<const block_type*>(address);

    if ((blockAddress >= &memoryPool[0]) && 
        (blockAddress <  &memoryPool[MemoryPoolSize]))
    {
        int blockIndex = blockAddress - memoryPool;
        int maskIndex = blockIndex / sizeof(mask_type);

        TRACE("Freeing blocks at %i\n", blockIndex);

        CACHE_PRELOAD_WRITE(&freeBlocksMask[maskIndex]);
        CACHE_PRELOAD_READ(&finalBlocksMask[maskIndex]);

        while(!isBlockSet(blockIndex, finalBlocksMask))
        {
            setBlock(blockIndex++, freeBlocksMask);
        }
        clearBlock(blockIndex, finalBlocksMask);
        
    }
    else
    {
        TRACE("Invalid free\n");
    }
}
