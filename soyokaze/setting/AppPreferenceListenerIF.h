#pragma once

class AppPreferenceListenerIF
{
public:
	virtual ~AppPreferenceListenerIF() {}

	virtual void OnAppFirstBoot() = 0;
	virtual void OnAppNormalBoot() = 0;
	virtual void OnAppPreferenceUpdated() = 0;
	virtual void OnAppExit() = 0;
};

