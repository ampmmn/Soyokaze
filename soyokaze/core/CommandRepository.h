#pragma once

#include "core/CommandIF.h"
#include "AppPreferenceListenerIF.h"
#include <vector>
#include <memory>

namespace soyokaze {
namespace core {

class CommandProvider;

class CommandRepository : public AppPreferenceListenerIF
{
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

	bool DeleteCommand(const CString& cmdName);

	void EnumCommands(std::vector<soyokaze::core::Command*>& commands);

	bool IsBuiltinName(const CString& cmdName);

	void Query(const CString& strQueryStr, std::vector<soyokaze::core::Command*>& commands);
	soyokaze::core::Command* QueryAsWholeMatch(const CString& strQueryStr, bool isSearchPath = true);

	bool IsValidAsName(const CString& strQueryStr);

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

