#include "pch.h"
#include "ExtraActionSettings.h"
#include "setting/AppPreferenceListenerIF.h"
#include "setting/AppPreference.h"

namespace launcherapp { namespace commands { namespace shellexecute {

struct ExtraActionSettings::PImpl : public AppPreferenceListenerIF
{
	PImpl()
	{
		mSettings.Load();
		AppPreference::Get()->RegisterListener(this);
	}
	virtual ~PImpl()
	{
		AppPreference::Get()->UnregisterListener(this);
	}

	void OnAppFirstBoot() override {}
	void OnAppNormalBoot() override {}
	void OnAppPreferenceUpdated() override
	{
		mSettings.Load();
	}
	void OnAppExit() override {}

	launcherapp::commands::explorepath::ExtraActionSettings mSettings;
};

ExtraActionSettings::ExtraActionSettings() : in(new PImpl)
{
}

ExtraActionSettings::~ExtraActionSettings()
{
}

ExtraActionSettings* ExtraActionSettings::Get()
{
	static ExtraActionSettings inst;
	return &inst;
}

launcherapp::commands::explorepath::ExtraActionSettings* ExtraActionSettings::GetSettings()
{
	return &in->mSettings;
}


}}}


