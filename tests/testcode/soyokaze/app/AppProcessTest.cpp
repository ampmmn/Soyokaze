#include "stdafx.h"
#include "gtest/gtest.h"
#include "app/AppProcess.h"
#include <regex>

static LPCTSTR PROCESS_MUTEX_NAME = _T("Global\\mutex_launcherapp_exist_for_test");

TEST(AppProcess, Instance)
{
	AppProcess p;
	EXPECT_TRUE(p.GetHandle() == nullptr);
}

TEST(AppProcess, IsExistSingle)
{
	AppProcess::SetSyncObjectName(PROCESS_MUTEX_NAME);
	AppProcess p;
	EXPECT_FALSE(p.Exists());
}

TEST(AppProcess, IsExistDouble)
{
	AppProcess::SetSyncObjectName(PROCESS_MUTEX_NAME);
	AppProcess p;
	EXPECT_FALSE(p.Exists());
	AppProcess p2;
	EXPECT_TRUE(p2.Exists());
}

TEST(AppProcess, Fail1)
{
	AppProcess::SetSyncObjectName(PROCESS_MUTEX_NAME);
	try {
		CEvent ev(FALSE, FALSE,PROCESS_MUTEX_NAME);
		AppProcess p;
		p.Exists();
		FAIL();
	}
	catch(AppProcess::exception&) {
		SUCCEED();
	}
}

