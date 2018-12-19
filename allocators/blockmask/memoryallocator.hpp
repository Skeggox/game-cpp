#pragma once

#include <cstddef>

class MemoryAllocator
{
    public:
        MemoryAllocator();
        ~MemoryAllocator();

        void* Allocate(const std::size_t size);
        void Free(const void* address);
   
    private:
        typedef std::size_t block_type;
        typedef std::size_t mask_type;

        // Tunable constants
        static const int MemoryPoolSize = 256 * 1024;

        // Calculated constants
        static const int BlockCount = MemoryPoolSize / sizeof(block_type);

        // Architecture-specific constants
        static const int CacheLineSize = 128;

        // Ensure both our memory pool and mask data is properly
        // cache-aligned
        alignas(CacheLineSize) block_type memoryPool[BlockCount];
        alignas(CacheLineSize) mask_type freeBlocksMask[BlockCount / sizeof(mask_type)];
        alignas(CacheLineSize) mask_type finalBlocksMask[BlockCount / sizeof(mask_type)];

        bool isBlockSet(int index, mask_type masks[]);
        void setBlock(int index, mask_type masks[]);
        void clearBlock(int index, mask_type masks[]);
};
