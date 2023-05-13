#pragma once

#include "CommandIF.h"
#include "AppPreferenceListenerIF.h"
#include <vector>

class CommandRepository : public AppPreferenceListenerIF
{
public:
	CommandRepository();
	virtual ~CommandRepository();

public:
	// コマンドデータのロード
	BOOL Load();

	// 新規登録ダイアログの表示
	int NewCommandDialog(const CString* cmdNamePtr, const CString* pathPtr, const CString* descStr = nullptr);
	// コマンド編集ダイアログの表示
	int EditCommandDialog(const CString& cmdName);
	// キーワードマネージャダイアログの表示
	int ManagerDialog();
	// まとめて登録ダイアログの表示
	int RegisterCommandFromFiles(const std::vector<CString>& files);

	bool DeleteCommand(const CString& cmdName);

	void EnumCommands(std::vector<Command*>& commands);

	bool IsBuiltinName(const CString& cmdName);

	void Query(const CString& strQueryStr, std::vector<Command*>& commands);
	Command* QueryAsWholeMatch(const CString& strQueryStr, bool isSearchPath = true);

	bool IsValidAsName(const CString& strQueryStr);

protected:
	virtual void OnAppPreferenceUpdated();

protected:
	struct PImpl;
	PImpl* in;
};

