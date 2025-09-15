#pragma once

#include "commands/core/CommandIF.h"
#include "commands/core/CommandQueryRequest.h"
#include "setting/AppPreferenceListenerIF.h"
#include <vector>
#include <memory>

class SettingPage;

namespace launcherapp {
namespace core {

class CommandProvider;

class CommandRepositoryListenerIF;

class CommandRepository : public AppPreferenceListenerIF
{
	using Parameter = launcherapp::actions::core::Parameter;
private:
	CommandRepository();
	virtual ~CommandRepository();

public:
	static CommandRepository* GetInstance();

	// コマンドプロバイダ登録
	void RegisterProvider(CommandProvider* provider);

	// コマンドを登録
	int RegisterCommand(Command* command);
	// コマンドの登録を解除
	int UnregisterCommand(Command* command);
	// 名前変更による登録しなおし
	int ReregisterCommand(Command* command);

	// コマンドデータのロード
	BOOL Load();

	// 新規登録ダイアログの表示
	int NewCommandDialog(Parameter* param = nullptr);

	// コマンド編集ダイアログの表示
	int EditCommandDialog(const CString& cmdName, bool isClone);
	// キーワードマネージャダイアログの表示
	int ManagerDialog();
	// まとめて登録ダイアログの表示
	int RegisterCommandFromFiles(const std::vector<CString>& files);

	void EnumCommands(std::vector<launcherapp::core::Command*>& commands);
	void EnumCommandDisplayNames(std::vector<CString>& displayNames);

	void Query(launcherapp::commands::core::CommandQueryRequest* req);
	launcherapp::core::Command* QueryAsWholeMatch(const CString& strQueryStr, bool isIncludeAdhocCommand = false);
	bool HasCommand(const CString& strQueryStr);

	bool IsValidAsName(const CString& strQueryStr);

	// クエリ要求の有無を返す
	bool HasQueryRequest();

	// リスナー登録
	void RegisterListener(CommandRepositoryListenerIF* listener);
	void UnregisterListener(CommandRepositoryListenerIF* listener);

	CString IssueClonedCommandName(const CString& baseName); 

protected:
	void RegisterHotKey(Command* command);

// AppPreferenceListenerIF
	void OnAppFirstBoot() override;
	void OnAppNormalBoot() override;
	void OnAppPreferenceUpdated() override;
	void OnAppExit() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}

