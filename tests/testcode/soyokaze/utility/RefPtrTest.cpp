#include "stdafx.h"
#include "gtest/gtest.h"
#include "utility/RefPtr.h"

namespace refptrtest {

class Countable
{
public:
	uint32_t AddRef() {
		return ++mRefCount;
	}
	uint32_t Release() {
		auto n = --mRefCount;
		return n;
	}

	uint32_t mRefCount = 1;
};

}

using namespace refptrtest;

TEST(RefPtr, construct0)
{
	RefPtr<Countable> ptr;
	EXPECT_TRUE(ptr.get() == nullptr);
}


TEST(RefPtr, constructNull)
{
	RefPtr<Countable> ptr;
	EXPECT_TRUE(ptr.get() == nullptr);
}

TEST(RefPtr, construct1)
{
	Countable obj;
	RefPtr<Countable> ptr(&obj);
	EXPECT_EQ(1, ptr->mRefCount);
	EXPECT_TRUE(ptr.get() != nullptr);
}

TEST(RefPtr, construct2)
{
	Countable obj;
	RefPtr<Countable> ptr(&obj, true);
	EXPECT_EQ(2, ptr->mRefCount);
}


TEST(RefPtr, moveConstruct)
{
	Countable obj;
	RefPtr<Countable> ptr(&obj);

	RefPtr<Countable> ptr2 = std::move(ptr);

	EXPECT_TRUE(ptr.get() == nullptr);
	EXPECT_TRUE(ptr2.get() != nullptr);
	EXPECT_EQ(1, ptr2->mRefCount);
}

TEST(RefPtr, copyConstruct)
{
	Countable obj;
	RefPtr<Countable> ptr(&obj);

	RefPtr<Countable> ptr2(ptr);

	EXPECT_TRUE(ptr.get() != nullptr);
	EXPECT_TRUE(ptr2.get() != nullptr);
	EXPECT_EQ(2, ptr->mRefCount);
	EXPECT_EQ(2, ptr2->mRefCount);
}

TEST(RefPtr, destruct)
{
	Countable obj;
	{
		RefPtr<Countable> ptr(&obj);
		RefPtr<Countable> ptr2(ptr);

		EXPECT_TRUE(ptr.get() != nullptr);
		EXPECT_TRUE(ptr2.get() != nullptr);
	}
	EXPECT_EQ(0, obj.mRefCount);
}

TEST(RefPtr, release)
{
	Countable obj;
	RefPtr<Countable> ptr(&obj);

	auto p = ptr.release();

	EXPECT_TRUE(&obj == p);
	EXPECT_TRUE(ptr.get() == nullptr);
}

TEST(RefPtr, reset)
{
	Countable obj;
	RefPtr<Countable> ptr(&obj);

	ptr.reset();

	EXPECT_TRUE(ptr.get() == nullptr);
	EXPECT_EQ(0, obj.mRefCount);
}

TEST(RefPtr, reset2)
{
	Countable obj;
	RefPtr<Countable> ptr(&obj);

	Countable obj2;
	ptr.reset(&obj2);

	EXPECT_TRUE(ptr.get() == &obj2);
	EXPECT_EQ(0, obj.mRefCount);
	EXPECT_EQ(1, obj2.mRefCount);
}

TEST(RefPtr, swap)
{
	Countable obj;
	RefPtr<Countable> ptr(&obj);

	Countable obj2;
	RefPtr<Countable> ptr2(&obj2);

	ptr.swap(ptr2);

	EXPECT_TRUE(ptr.get() == &obj2);
	EXPECT_TRUE(ptr2.get() == &obj);
}

