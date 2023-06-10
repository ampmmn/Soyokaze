#pragma once

#include "AppPreferenceListenerIF.h"
#include "Settings.h"
#include <memory>

class CommandHotKeyMappings;

class AppPreference
{
	class NotifyWindow;

public:
	static AppPreference* Get();

	void Init();

	void Load();
	void Save();
	void OnExit();

	void SetSettings(const Settings& settings);
	const Settings& GetSettings();

	CString GetFilerPath();
	CString GetFilerParam();
	bool IsUseFiler();

	bool IsHideOnStartup();
	bool IsHideOnInactive();

	int GetMatchLevel();
	bool IsTopMost();
	bool IsShowToggle();

	bool IsWindowTransparencyEnable();
	int GetAlpha();
	bool IsTransparencyInactiveOnly();

	UINT GetModifiers();
	UINT GetVirtualKeyCode();

	// 入力画面表示時にIMEをオフにするか?
	bool IsIMEOffOnActive();

	void SetCommandKeyMappings(const CommandHotKeyMappings& keyMap);
	void GetCommandKeyMappings(CommandHotKeyMappings& keyMap);

	void RegisterListener(AppPreferenceListenerIF* listener);
	void UnregisterListener(AppPreferenceListenerIF* listener);

	bool CreateUserDirectory();

protected:
	AppPreference();
	~AppPreference();

public:
	struct PImpl;
	std::unique_ptr<PImpl> in;

};

