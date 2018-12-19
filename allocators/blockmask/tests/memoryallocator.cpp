#include "memoryallocator.hpp"

CPPUNIT_TEST_SUITE_REGISTRATION( MemoryAllocatorTest );

void MemoryAllocatorTest::setUp()
{
    pAllocator = new MemoryAllocator();
}

void MemoryAllocatorTest::tearDown()
{
    delete pAllocator;
    pAllocator = nullptr;
}

void MemoryAllocatorTest::testSingle()
{
    int * intPtr = nullptr;

    intPtr = static_cast<int*>(pAllocator->Allocate(sizeof(int)));

    CPPUNIT_ASSERT(intPtr != nullptr);

    *intPtr = 0;
    
    CPPUNIT_ASSERT(*intPtr == 0);

    *intPtr = 1;
    
    CPPUNIT_ASSERT(*intPtr == 1);
    
    pAllocator->Free(intPtr);
}


void MemoryAllocatorTest::testMultiple()
{
    size_t sizes[4] = { 256, 1021, 76, 513 };
    void* ptrs[4];

    for(int i = 0; i < 4; ++i)
    {
        ptrs[i] = pAllocator->Allocate(sizes[i]);
        CPPUNIT_ASSERT(ptrs[i] != nullptr);
    }

    for(int i = 0; i < 4; ++i)
    {
        pAllocator->Free(ptrs[i]);
    }

    for(size_t size : sizes)
    {
        void* ptr = pAllocator->Allocate(size);
        CPPUNIT_ASSERT(ptr != nullptr);
        pAllocator->Free(ptr);
    }
}

void MemoryAllocatorTest::testTooLarge()
{
    void *ptr = pAllocator->Allocate(513 * 1024);
    CPPUNIT_ASSERT(ptr == nullptr);
}

void MemoryAllocatorTest::testInvalidFree()
{
    void *ptr = nullptr;
    pAllocator->Free(ptr);
}

void MemoryAllocatorTest::testMultiFree()
{
    void *ptr = pAllocator->Allocate(1024);
    CPPUNIT_ASSERT(ptr != nullptr);
    pAllocator->Free(ptr);
    pAllocator->Free(ptr);
}
