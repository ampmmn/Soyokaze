#include "stdafx.h"
#include "gtest/gtest.h"
#include "logger/ResourceUsageMetrics.h"

using namespace logger;

TEST(ResourceUsageMetricsTest, WorkingSetMetricsTest)
{
	WorkingSetMetrics metrics;

	EXPECT_EQ(metrics.GetOrder(), 10);
	EXPECT_EQ(metrics.GetName(), "WorkingSet");
	EXPECT_FALSE(metrics.GetValue().empty());
}

TEST(ResourceUsageMetricsTest, PrivateBytesMetricsTest)
{
	PrivateBytesMetrics metrics;

	EXPECT_EQ(metrics.GetOrder(), 20);
	EXPECT_EQ(metrics.GetName(), "PrivateBytes");
	EXPECT_FALSE(metrics.GetValue().empty());
}

TEST(ResourceUsageMetricsTest, ThreadMetricsTest)
{
	ThreadMetrics metrics;

	EXPECT_EQ(metrics.GetOrder(), 30);
	EXPECT_EQ(metrics.GetName(), "Threads");
	EXPECT_FALSE(metrics.GetValue().empty());
}

TEST(ResourceUsageMetricsTest, GdiObjectsMetricsTest)
{
	GdiObjectsMetrics metrics;

	EXPECT_EQ(metrics.GetOrder(), 40);
	EXPECT_EQ(metrics.GetName(), "GDI Objects");
	EXPECT_FALSE(metrics.GetValue().empty());
}

TEST(ResourceUsageMetricsTest, UserObjectsMetricsTest)
{
	UserObjectsMetrics metrics;

	EXPECT_EQ(metrics.GetOrder(), 50);
	EXPECT_EQ(metrics.GetName(), "User Objects");
	EXPECT_FALSE(metrics.GetValue().empty());
}
