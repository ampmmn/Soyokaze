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
	EXPECT_FALSE(p.IsExist());
}

TEST(AppProcess, IsExistDouble)
{
	AppProcess::SetSyncObjectName(PROCESS_MUTEX_NAME);
	AppProcess p;
	EXPECT_FALSE(p.IsExist());
	AppProcess p2;
	EXPECT_TRUE(p2.IsExist());
}

TEST(AppProcess, Fail1)
{
	AppProcess::SetSyncObjectName(PROCESS_MUTEX_NAME);
	try {
		CEvent ev(FALSE, FALSE,PROCESS_MUTEX_NAME);
		AppProcess p;
		p.IsExist();
		FAIL();
	}
	catch(AppProcess::exception&) {
		SUCCEED();
	}
}

