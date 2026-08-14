#include "stdafx.h"
#include "gtest/gtest.h"
#include "externaltool/webbrowser/BrowserProfile.h"
#include "utility/Path.h"

#include <filesystem>
#include <fstream>

using BrowserProfile = launcherapp::externaltool::webbrowser::BrowserProfile;

namespace {

class BrowserProfileTest : public testing::Test
{
protected:
	void SetUp() override
	{
		TCHAR tempPath[MAX_PATH];
		GetTempPath(MAX_PATH, tempPath);
		GetTempFileName(tempPath, _T("Soy"), 0, tempPath);
		DeleteFile(tempPath);
		CreateDirectory(tempPath, nullptr);
		mUserDataPath = tempPath;
	}

	void TearDown() override
	{
		Path localStatePath(mUserDataPath);
		localStatePath.Append(_T("Local State"));
		DeleteFile(localStatePath);
		RemoveDirectory(mUserDataPath);
	}

	void WriteLocalState(const char* content)
	{
		Path localStatePath(mUserDataPath);
		localStatePath.Append(_T("Local State"));
		std::ofstream file{std::filesystem::path((LPCTSTR)localStatePath)};
		file << content;
	}

	CString mUserDataPath;
};

}

TEST_F(BrowserProfileTest, GetLastUsedProfileName)
{
	WriteLocalState(R"({"profile":{"last_used":"Profile 1"}})");

	EXPECT_STREQ(_T("Profile 1"), BrowserProfile::GetLastUsedProfileName(mUserDataPath));
}

TEST_F(BrowserProfileTest, UseDefaultWhenLocalStateDoesNotExist)
{
	EXPECT_STREQ(_T("Default"), BrowserProfile::GetLastUsedProfileName(mUserDataPath));
}

TEST_F(BrowserProfileTest, UseDefaultWhenLocalStateIsInvalid)
{
	WriteLocalState("invalid json");

	EXPECT_STREQ(_T("Default"), BrowserProfile::GetLastUsedProfileName(mUserDataPath));
}

TEST_F(BrowserProfileTest, UseDefaultWhenLastUsedIsMissing)
{
	WriteLocalState(R"({"profile":{}})");

	EXPECT_STREQ(_T("Default"), BrowserProfile::GetLastUsedProfileName(mUserDataPath));
}
