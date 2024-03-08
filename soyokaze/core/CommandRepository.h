#pragma once

#include "core/CommandIF.h"
#include "AppPreferenceListenerIF.h"
#include <vector>
#include <memory>

class SettingPage;

namespace soyokaze {
namespace core {

class CommandProvider;

class CommandRepositoryListenerIF;

class CommandRepository : public AppPreferenceListenerIF
{
private:
	CommandRepository();
	virtual ~CommandRepository();

public:
	static CommandRepository* GetInstance();

	// コマンドプロバイダ登録
	void RegisterProvider(CommandProvider* provider);

	// コマンドプロバイダの設定ページを列挙する
	void EnumProviderSettingDialogs(CWnd* parent, std::vector<SettingPage*>& pages);

	// コマンドを登録
	int RegisterCommand(Command* command);
	// コマンドの登録を解除
	int UnregisterCommand(Command* command);
	// 名前変更による登録しなおし
	int ReregisterCommand(Command* command);

	// 優先度の更新
	void AddRank(Command* command, int number);

	// コマンドデータのロード
	BOOL Load();

	// 新規登録ダイアログの表示
	int NewCommandDialog(const CommandParameter* param = nullptr);

	// コマンド編集ダイアログの表示
	int EditCommandDialog(const CString& cmdName);
	// キーワードマネージャダイアログの表示
	int ManagerDialog();
	// まとめて登録ダイアログの表示
	int RegisterCommandFromFiles(const std::vector<CString>& files);

	void EnumCommands(std::vector<soyokaze::core::Command*>& commands);

	bool IsBuiltinName(const CString& cmdName);

	void Query(const CommandParameter& param, std::vector<soyokaze::core::Command*>& commands);
	soyokaze::core::Command* QueryAsWholeMatch(const CString& strQueryStr, bool isIncludeAdhocCommand = false);
	bool HasCommand(const CString& strQueryStr);

	bool IsValidAsName(const CString& strQueryStr);

	// リスナー登録
	void RegisterListener(CommandRepositoryListenerIF* listener);
	void UnregisterListener(CommandRepositoryListenerIF* listener);

protected:
	void OnAppFirstBoot() override;
	void OnAppPreferenceUpdated() override;
	void OnAppExit() override;

protected:
	struct PImpl;
	std::unique_ptr<PImpl> in;
};


}
}

