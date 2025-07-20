#pragma once

#include "commands/core/CommandIF.h"
#include "commands/core/CommandQueryItemList.h"
#include <map>
#include <vector>

class CommandMap
{
public:
	using CommandQueryItem = launcherapp::CommandQueryItem;
	using CommandQueryItemList = launcherapp::CommandQueryItemList;

	class Settings;

public:
	CommandMap();
	CommandMap(const CommandMap& rhs);
	~CommandMap();

	void LoadSettings(Settings& settings);
	void RestoreSettings(Settings& settings);

	void Clear();

	// コマンドオブジェクトに紐づけられた名前を問い合わせる
	bool QueryRegisteredNameFor(launcherapp::core::Command* targetCmd, CString& registeredName);

	bool Has(const CString& name) const;
	launcherapp::core::Command* Get(const CString& name);

	// 登録/解除
	void Register(launcherapp::core::Command* cmd);
	bool Unregister(launcherapp::core::Command* cmd);
	bool Unregister(const CString& name);
	// リネーム
	bool Reregister(launcherapp::core::Command* cmd);

	void Swap(CommandMap& rhs);

	void Query(Pattern* pattern, CommandQueryItemList& commands);

	// 最初に見つけた要素を返す
	launcherapp::core::Command* FindOne(Pattern* pattern);

	// 配列化する
	std::vector<launcherapp::core::Command*>& Enumerate(std::vector<launcherapp::core::Command*>& commands);

protected:
	std::map<CString, launcherapp::core::Command*> mMap;
};


class CommandMap::Settings
{
public:
	Settings();
	~Settings();

	void Add(launcherapp::core::Command* cmd);
	bool Restore(launcherapp::core::Command* cmd);

private:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};

