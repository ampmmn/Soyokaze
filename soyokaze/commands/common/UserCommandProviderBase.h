#pragma once

#include "commands/core/CommandProviderIF.h"
#include "commands/core/CommandFile.h"

namespace launcherapp {
namespace commands {
namespace common {

class UserCommandProviderBase :
	public launcherapp::core::CommandProvider
{
protected:
	using Command = launcherapp::core::Command;
	using CommandParameter = launcherapp::core::CommandParameter;

protected:
	UserCommandProviderBase();
	~UserCommandProviderBase() override;

public:
	// 初回起動の初期化を行う
	void OnFirstBoot() override;

	// コマンドの読み込み
	void LoadCommands(CommandFile* commandFile) override;

	// 非公開コマンドかどうか(新規作成対象にしない)
	bool IsPrivate() const override;

	void QueryAdhocCommands(Pattern* pattern, CommandQueryItemList& comands) override;

	bool CreateSettingPages(CWnd* parent, std::vector<SettingPage*>& pages) override;

	uint32_t AddRef() override;
	uint32_t Release() override;

	// 派生クラス側で実装する必要のあるメソッド
	virtual bool LoadFrom(CommandEntryIF* entry, Command** command);

private:
	ULONGLONG mRefCount;
};

} // end of namespace common
} // end of namespace commands
} // end of namespace launcherapp


#define DECLARE_LOADFROM(providerClsName) \
	bool LoadFrom(CommandEntryIF* entry, Command** command) override;

#define IMPLEMENT_LOADFROM(providerClsName, commandClsName) \
	bool providerClsName::LoadFrom(CommandEntryIF* entry, Command** retCommand) { \
		std::unique_ptr<commandClsName> command(new commandClsName); \
		if (command->Load(entry) == false) { return false; } \
		*retCommand = command.release(); \
		return true; \
	}


