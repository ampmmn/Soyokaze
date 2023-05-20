#pragma once

#include "AppPreferenceListenerIF.h"
#include "Settings.h"
#include <set>

class AppPreference
{
public:
	static AppPreference* Get();

	void Load();
	void Save();

	void SetSettings(const Settings& settings);
	const Settings& GetSettings();

	CString GetFilerPath();
	CString GetFilerParam();
	bool IsUseFiler();

	bool IsHideOnStartup();

	int GetMatchLevel();
	bool IsTopMost();
	bool IsShowToggle();

	bool IsWindowTransparencyEnable();
	int GetAlpha();
	bool IsTransparencyInactiveOnly();

	UINT GetModifiers();
	UINT GetVirtualKeyCode();

	void RegisterListener(AppPreferenceListenerIF* listener);
	void UnregisterListener(AppPreferenceListenerIF* listener);

protected:
	AppPreference();
	~AppPreference();

public:
	Settings mSettings;

protected:
	// 設定変更時(正確にはSave時)に通知を受け取る
	std::set<AppPreferenceListenerIF*> mListeners;
};

