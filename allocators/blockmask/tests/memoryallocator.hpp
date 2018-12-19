#pragma once
#include "..\memoryallocator.hpp"
#include <cppunit/extensions/HelperMacros.h>

class MemoryAllocatorTest : public CppUnit::TestFixture
{
  CPPUNIT_TEST_SUITE( MemoryAllocatorTest );
  
  CPPUNIT_TEST( testSingle );
  CPPUNIT_TEST( testMultiple );
  CPPUNIT_TEST( testTooLarge );
  CPPUNIT_TEST( testInvalidFree );
  CPPUNIT_TEST( testMultiFree );
  
  CPPUNIT_TEST_SUITE_END();

  MemoryAllocator* pAllocator;

public:
  void setUp();
  void tearDown();

  void testSingle();
  void testMultiple();
  void testTooLarge();
  void testInvalidFree();
  void testMultiFree();
};
