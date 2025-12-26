#pragma once

#include "hotkey/HotKeyAttribute.h"

class Settings;

namespace launcherapp { namespace commands { namespace explorepath {

class ExtraActionSettings
{
public:

	class Entry {
	public:
		// ラベル
		CString mLabel;
		// 実行するコマンド名
		CString mCommand;
		// キー設定
		HOTKEY_ATTR mHotkeyAttr;
		// ファイルに対して有効か?
		bool mIsForFile{true};
		// フォルダに対して有効か?
		bool mIsForFolder{false};
	};

public:
	ExtraActionSettings();
	~ExtraActionSettings();

	void Save(Settings* settingsPtr);
	void Save();
	void Load();

	int GetEntryCount();
	bool GetEntry(size_t index, ExtraActionSettings::Entry& entry);

	void SwapEntries(std::vector<Entry>& entries);

private:
	std::vector<Entry> mEntries;
};

}}}

