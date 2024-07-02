#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/GlobalALlocMemory.h"

TEST(GlobalAllocMemory, testConstruct)
{
	GlobalAllocMemory mem(32);
	EXPECT_TRUE(mem != (HGLOBAL)nullptr);
}

TEST(GlobalAllocMemory, testConstruct2)
{
	GlobalAllocMemory mem(0);
	EXPECT_TRUE(mem != (HGLOBAL)nullptr);
}


TEST(GlobalAllocMemory, testLock)
{
	GlobalAllocMemory mem(32);
	EXPECT_TRUE(mem.Lock() != nullptr);
	mem.Unlock();
}


TEST(GlobalAllocMemory, testRelease)
{
	GlobalAllocMemory mem(32);
	HGLOBAL h = mem.Release();
	EXPECT_TRUE(h != nullptr);
	GlobalFree(h);
}


