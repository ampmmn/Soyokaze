#include "pch.h"
#include "ColorSettings.h"
#include "gui/SystemColorScheme.h"
#include "gui/CustomColorScheme.h"
#include "setting/AppPreference.h"
#include "setting/AppPreferenceListenerIF.h"


#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct ColorSettings::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		AppPreference::Get()->RegisterListener(this);
	}

	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		auto pref = AppPreference::Get();
		mIsUseSystemSettings = pref->IsUseSystemColorSettings();
		mCustomColor.Reload();
	}
	void OnAppExit() override
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	bool mIsUseSystemSettings = true;
	SystemColorScheme mSysColor;
	CustomColorScheme mCustomColor;
};


ColorSettings::ColorSettings() : in(new PImpl)
{
	auto pref = AppPreference::Get();
	in->mIsUseSystemSettings = pref->IsUseSystemColorSettings();
}

ColorSettings::~ColorSettings()
{
}

ColorSettings* ColorSettings::Get()
{
	static ColorSettings inst;
	return &inst;
}

bool ColorSettings::IsSystemSettings()
{
	return in->mIsUseSystemSettings;
}

ColorSchemeIF* ColorSettings::GetCurrentScheme()
{
	if (in->mIsUseSystemSettings) {
		return &in->mSysColor;
	}
	else {
		return &in->mCustomColor;
	}
}

