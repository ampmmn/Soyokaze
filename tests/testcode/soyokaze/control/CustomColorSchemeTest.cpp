#include "stdafx.h"
#include "gtest/gtest.h"
#include "control/CustomColorScheme.h"
#include "setting/AppPreference.h"
#include "setting/Settings.h"
#include <memory>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace {

class ScopedAppSettings
{
public:
	/** 現在のアプリ設定を保存する */
	ScopedAppSettings() : mPreference(AppPreference::Get())
	{
		mPreference->GetListTextColor();
		mOriginalSettings.reset(mPreference->GetSettings().Clone());
	}

	/** 保存したアプリ設定を復元する */
	~ScopedAppSettings()
	{
		mPreference->SetSettings(*mOriginalSettings);
	}

	/** テスト用のアプリ設定を適用する */
	void Set(const Settings& settings)
	{
		mPreference->SetSettings(settings);
	}

private:
	AppPreference* mPreference;
	std::unique_ptr<Settings> mOriginalSettings;
};

}

TEST(CustomColorScheme, DefaultsSecondLineColorsToFirstLineAndLoadsOverrides)
{
	ScopedAppSettings appSettings;
	Settings settings;
	COLORREF textColor = RGB(20, 40, 60);
	COLORREF highlightTextColor = RGB(80, 100, 120);
	settings.Set(_T("CurrentColor:ListText"), static_cast<int>(textColor));
	settings.Set(_T("CurrentColor:ListHighlightText"), static_cast<int>(highlightTextColor));
	appSettings.Set(settings);

	CustomColorScheme colorScheme;
	EXPECT_EQ(textColor, colorScheme.GetListText2Color());
	EXPECT_EQ(highlightTextColor, colorScheme.GetListHighlightText2Color());

	COLORREF text2Color = RGB(140, 160, 180);
	COLORREF highlightText2Color = RGB(200, 220, 240);
	settings.Set(_T("CurrentColor:ListText2"), static_cast<int>(text2Color));
	settings.Set(_T("CurrentColor:ListHighlightText2"), static_cast<int>(highlightText2Color));
	appSettings.Set(settings);
	colorScheme.Reload();

	EXPECT_EQ(text2Color, colorScheme.GetListText2Color());
	EXPECT_EQ(highlightText2Color, colorScheme.GetListHighlightText2Color());
}
