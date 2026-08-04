#include "stdafx.h"
#include "gtest/gtest.h"
#include "logger/ResourceUsageMetrics.h"
#include "icon/IconLoaderIconIndexCachePathCountMetrics.h"
#include "icon/IconLoaderIconIndexCacheIconCountMetrics.h"
#include "icon/IconLoaderDefaultIconCacheCountMetrics.h"
#include "icon/IconLoaderFileExtIconCacheCountMetrics.h"
#include "icon/IconLoaderAppIconCacheCountMetrics.h"
#include "icon/IconLoaderManagedIconCountMetrics.h"
#include "commands/core/CommandProviderCountMetrics.h"
#include "commands/core/CommandRepositoryCommandCountMetrics.h"
#include "commands/core/CommandRepositoryQueryRequestCountMetrics.h"
#include "commands/core/CommandRepositoryListenerCountMetrics.h"
#include "mainwindow/CandidateListCandidateCountMetrics.h"
#include "mainwindow/CandidateListListenerCountMetrics.h"
#include "mainwindow/CandidateListHasErrorCommandMetrics.h"

using namespace logger;

template <typename Metrics>
void ExpectMetrics(Metrics& metrics, int order, const char* name)
{
	EXPECT_EQ(metrics.GetOrder(), order);
	EXPECT_EQ(metrics.GetName(), name);
	EXPECT_FALSE(metrics.GetValue().empty());
}

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

TEST(ResourceUsageMetricsTest, AdditionalMetricsTest)
{
	IconLoaderIconIndexCachePathCountMetrics iconIndexPathMetrics;
	ExpectMetrics(iconIndexPathMetrics, 100, "IconLoaderIconIndexCachePathCount");

	IconLoaderIconIndexCacheIconCountMetrics iconIndexIconMetrics;
	ExpectMetrics(iconIndexIconMetrics, 110, "IconLoaderIconIndexCacheIconCount");

	IconLoaderDefaultIconCacheCountMetrics defaultIconMetrics;
	ExpectMetrics(defaultIconMetrics, 120, "IconLoaderDefaultIconCacheCount");

	IconLoaderFileExtIconCacheCountMetrics fileExtIconMetrics;
	ExpectMetrics(fileExtIconMetrics, 130, "IconLoaderFileExtIconCacheCount");

	IconLoaderAppIconCacheCountMetrics appIconMetrics;
	ExpectMetrics(appIconMetrics, 140, "IconLoaderAppIconCacheCount");

	IconLoaderManagedIconCountMetrics managedIconMetrics;
	ExpectMetrics(managedIconMetrics, 150, "IconLoaderManagedIconCount");

	CommandProviderCountMetrics providerMetrics;
	ExpectMetrics(providerMetrics, 200, "CommandProviderCount");

	CommandRepositoryCommandCountMetrics commandMetrics;
	ExpectMetrics(commandMetrics, 300, "CommandRepositoryCommandCount");

	CommandRepositoryQueryRequestCountMetrics queryRequestMetrics;
	ExpectMetrics(queryRequestMetrics, 310, "CommandRepositoryQueryRequestCount");

	CommandRepositoryListenerCountMetrics repositoryListenerMetrics;
	ExpectMetrics(repositoryListenerMetrics, 320, "CommandRepositoryListenerCount");

	CandidateListCandidateCountMetrics candidateMetrics;
	ExpectMetrics(candidateMetrics, 400, "CandidateListCandidateCount");

	CandidateListListenerCountMetrics candidateListenerMetrics;
	ExpectMetrics(candidateListenerMetrics, 410, "CandidateListListenerCount");

	CandidateListHasErrorCommandMetrics errorCommandMetrics;
	ExpectMetrics(errorCommandMetrics, 420, "CandidateListHasErrorCommand");
}
