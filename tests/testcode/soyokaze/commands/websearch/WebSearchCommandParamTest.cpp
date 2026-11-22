#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/websearch/WebSearchCommandParam.h"

namespace {

class MockWebSearchCommandEntry : public CommandEntryIF
{
public:
	CString mName;
	std::map<CString, CString> mStringValues;
	std::map<CString, bool> mBoolValues;

	LPCTSTR GetName() override { return mName; }
	void MarkAsUsed() override {}
	bool IsUsedEntry() override { return false; }
	bool HasValue(LPCTSTR key) override { return mStringValues.count(key) > 0 || mBoolValues.count(key) > 0; }
	int GetValueType(LPCTSTR) override { return 0; }
	int Get(LPCTSTR, int defaultValue) override { return defaultValue; }
	void Set(LPCTSTR, int) override {}
	double Get(LPCTSTR, double defaultValue) override { return defaultValue; }
	void Set(LPCTSTR, double) override {}
	CString Get(LPCTSTR key, LPCTSTR defaultValue) override
	{
		auto it = mStringValues.find(key);
		return it == mStringValues.end() ? CString(defaultValue) : it->second;
	}
	void Set(LPCTSTR key, const CString& value) override { mStringValues[key] = value; }
	bool Get(LPCTSTR key, bool defaultValue) override
	{
		auto it = mBoolValues.find(key);
		return it == mBoolValues.end() ? defaultValue : it->second;
	}
	void Set(LPCTSTR key, bool value) override { mBoolValues[key] = value; }
	size_t GetBytesLength(LPCTSTR) override { return CommandEntryIF::NO_ENTRY; }
	bool GetBytes(LPCTSTR, uint8_t*, size_t) override { return false; }
	void SetBytes(LPCTSTR, const uint8_t*, size_t) override {}
	bool DumpRawData(std::vector<uint8_t>&) override { return false; }
	uint32_t AddRef() override { return 1; }
	uint32_t Release() override { return 1; }
};

using CommandParam = launcherapp::commands::websearch::CommandParam;

}

TEST(WebSearchCommandParamTest, SearchViewerSettingsSaveAndLoad)
{
	CommandParam param;
	param.mName = _T("web");
	param.mIsUseSearchViewer = true;
	param.mSearchViewerTitle = _T("検索結果");

	MockWebSearchCommandEntry entry;
	param.Save(&entry);

	CommandParam loaded;
	loaded.Load(&entry);

	EXPECT_TRUE(loaded.mIsUseSearchViewer);
	EXPECT_EQ(_T("検索結果"), loaded.mSearchViewerTitle);
}

TEST(WebSearchCommandParamTest, OldEntryDefaultsToSystemBrowser)
{
	MockWebSearchCommandEntry entry;
	entry.mName = _T("web");

	CommandParam loaded;
	loaded.Load(&entry);

	EXPECT_FALSE(loaded.mIsUseSearchViewer);
	EXPECT_TRUE(loaded.mSearchViewerTitle.IsEmpty());
}
