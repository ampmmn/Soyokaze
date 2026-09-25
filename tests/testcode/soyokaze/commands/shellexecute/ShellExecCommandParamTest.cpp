#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/shellexecute/ShellExecCommandParam.h"

using CommandParam = launcherapp::commands::shellexecute::CommandParam;

class MockShellExecCommandEntry : public CommandEntryIF
{
public:
	CString mName;
	std::map<CString, bool> mBoolValues;
	std::map<CString, CString> mStringValues;

	LPCTSTR GetName() override { return mName; }
	void MarkAsUsed() override {}
	bool IsUsedEntry() override { return false; }
	bool HasValue(LPCTSTR key) override { return mBoolValues.count(key) > 0 || mStringValues.count(key) > 0; }
	int GetValueType(LPCTSTR) override { return 0; }
	int Get(LPCTSTR, int defaultValue) override { return defaultValue; }
	void Set(LPCTSTR, int) override {}
	double Get(LPCTSTR, double defaultValue) override { return defaultValue; }
	void Set(LPCTSTR, double) override {}
	CString Get(LPCTSTR key, LPCTSTR defaultValue) override
	{
		auto iter = mStringValues.find(key);
		return iter != mStringValues.end() ? iter->second : CString(defaultValue);
	}
	void Set(LPCTSTR key, const CString& value) override { mStringValues[key] = value; }
	bool Get(LPCTSTR key, bool defaultValue) override
	{
		auto iter = mBoolValues.find(key);
		return iter != mBoolValues.end() ? iter->second : defaultValue;
	}
	void Set(LPCTSTR key, bool value) override { mBoolValues[key] = value; }
	size_t GetBytesLength(LPCTSTR) override { return CommandEntryIF::NO_ENTRY; }
	bool GetBytes(LPCTSTR, uint8_t*, size_t) override { return false; }
	void SetBytes(LPCTSTR, const uint8_t*, size_t) override {}
	bool DumpRawData(std::vector<uint8_t>&) override { return false; }
	uint32_t AddRef() override { return 1; }
	uint32_t Release() override { return 1; }
};

TEST(ShellExecCommandParamTest, ExtraCandidateIsEnabledByDefault)
{
	CommandParam param;

	EXPECT_TRUE(param.mIsUseExtraCandidate);
}

TEST(ShellExecCommandParamTest, SaveAndLoadExtraCandidateSetting)
{
	CommandParam param;
	param.mName = _T("sample");
	param.mIsUseExtraCandidate = FALSE;

	MockShellExecCommandEntry entry;
	param.Save(&entry);

	EXPECT_FALSE(entry.mBoolValues[_T("use_extracandidate")]);

	CommandParam loaded;
	loaded.Load(&entry);
	EXPECT_FALSE(loaded.mIsUseExtraCandidate);
}

TEST(ShellExecCommandParamTest, LegacyEntryEnablesExtraCandidate)
{
	MockShellExecCommandEntry entry;
	CommandParam loaded;

	loaded.Load(&entry);

	EXPECT_TRUE(loaded.mIsUseExtraCandidate);
}

TEST(ShellExecCommandParamTest, CopyPreservesExtraCandidateSetting)
{
	CommandParam param;
	param.mIsUseExtraCandidate = FALSE;

	CommandParam copied(param);
	EXPECT_FALSE(copied.mIsUseExtraCandidate);

	CommandParam assigned;
	assigned = param;
	EXPECT_FALSE(assigned.mIsUseExtraCandidate);
}
