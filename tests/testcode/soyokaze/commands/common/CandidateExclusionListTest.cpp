#include "stdafx.h"
#include "gtest/gtest.h"
#include "commands/common/CandidateExclusionList.h"
#include "setting/AppPreference.h"
#include "setting/Settings.h"
#include <memory>

namespace {

class ScopedAppSettings
{
public:
	ScopedAppSettings() :
		mPreference(AppPreference::Get()),
		mOriginalSettings(mPreference->GetSettings().Clone())
	{
	}

	~ScopedAppSettings()
	{
		mPreference->SetSettings(*mOriginalSettings);
	}

	void Set(const Settings& settings)
	{
		mPreference->SetSettings(settings);
	}

private:
	AppPreference* mPreference;
	std::unique_ptr<Settings> mOriginalSettings;
};

}

TEST(CandidateExclusionListTest, LoadsPathsAndCaseInsensitivePartialDisplayNamePatterns)
{
	ScopedAppSettings appSettings;
	Settings settings;
	settings.Set(_T("Soyokaze:ExcludePathCount"), 1);
	settings.Set(_T("Soyokaze:ExcludePath1"), CString(_T("C:\\Apps\\Legacy.exe")));
	settings.Set(_T("ExcludeTarget:PatternCount"), 2);
	settings.Set(_T("ExcludeTarget:Pattern0"), CString(_T("365\\s+copilot")));
	settings.Set(_T("ExcludeTarget:Pattern1"), CString(_T("(")));
	appSettings.Set(settings);
	EXPECT_EQ(1, AppPreference::Get()->GetSettings().Get(_T("Soyokaze:ExcludePathCount"), 0));
	EXPECT_EQ(2, AppPreference::Get()->GetSettings().Get(_T("ExcludeTarget:PatternCount"), 0));
	EXPECT_STREQ(_T("C:\\Apps\\Legacy.exe"), AppPreference::Get()->GetSettings().Get(_T("Soyokaze:ExcludePath1"), _T("")));
	EXPECT_STREQ(_T("365\\s+copilot"), AppPreference::Get()->GetSettings().Get(_T("ExcludeTarget:Pattern0"), _T("")));

	launcherapp::commands::common::CandidateExclusionList list;
	list.Load();

	EXPECT_TRUE(list.IsExcludedPath(_T("c:\\apps\\legacy.exe")));
	EXPECT_FALSE(list.IsExcludedPath(_T("C:\\Apps\\Other.exe")));
	EXPECT_TRUE(list.IsExcludedDisplayName(_T("Microsoft 365 Copilot")));
	EXPECT_TRUE(list.IsExcludedDisplayName(_T("Microsoft 365 Copilot Preview")));
	EXPECT_FALSE(list.IsExcludedDisplayName(_T("Microsoft 365 Teams")));
}
