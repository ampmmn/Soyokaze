#include "pch.h"
#include "CommandRepository.h"
#include "CommandMap.h"
#include "gui/AboutDlg.h"
#include "utility/AppProfile.h"
#include "AppPreference.h"
#include "PartialMatchPattern.h"
#include "WholeMatchPattern.h"
#include "core/CommandProviderIF.h"
#include "CommandFile.h"
#include "commands/shellexecute/ShellExecCommand.h"
#include "gui/KeywordManagerDialog.h"
#include "gui/SelectFilesDialog.h"
#include "gui/SelectCommandTypeDialog.h"
#include "core/CommandHotKeyManager.h"
#include "core/CommandRanking.h"
#include "CommandHotKeyMappings.h"
#include "NamedCommandHotKeyHandler.h"
#include <vector>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using ShellExecCommand = soyokaze::commands::shellexecute::ShellExecCommand;

namespace soyokaze {
namespace core {

struct CommandRepository::PImpl
{
	void ReloadPatternObject()
	{
		mPattern.reset(new PartialMatchPattern());

		// コマンドのホットキー設定のリロード
		CommandHotKeyMappings hotKeyMap;

		auto pref = AppPreference::Get();
		pref->GetCommandKeyMappings(hotKeyMap);

		auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
		hotKeyManager->Clear();

		int count = hotKeyMap.GetItemCount();
		for (int i = 0; i < count; ++i) {
			auto name = hotKeyMap.GetName(i);
			HOTKEY_ATTR attr;
			hotKeyMap.GetHotKeyAttr(i, attr);

			auto handler = std::make_unique<NamedCommandHotKeyHandler>(name);
			bool isGlobal = hotKeyMap.IsGlobal(i);

			hotKeyManager->Register(handler.release(), attr, isGlobal);
		}
	}

	void SaveCommands()
	{
		CommandFile commandFile;
		commandFile.SetFilePath(mCommandFilePath);

		std::vector<soyokaze::core::Command*> commands;
		for (auto& command : mCommands.Enumerate(commands)) {
			command->Save(&commandFile);
			command->Release();
		}
		commandFile.Save();
	}

	CCriticalSection mCS;

	std::vector<CommandProvider*> mProviders;

	// 設定ファイル(commands.ini)のパス
	CString mCommandFilePath;

	// 一般コマンド一覧
	CommandMap mCommands;
	// キーワード比較用のクラス
	std::unique_ptr<Pattern> mPattern;
	
	// 優先順位
	CommandRanking mRanking;

	// 編集中フラグ
	bool mIsNewDialog = false;
	bool mIsEditDialog = false;
	bool mIsManagerDialog = false;
	bool mIsRegisteFromFileDialog = false;

	// 再入防止フラグ
	bool mIsQuering = false;
};


CommandRepository::CommandRepository() : in(std::make_unique<PImpl>())
{
	AppPreference::Get()->RegisterListener(this);

	TCHAR path[MAX_PATH_NTFS];
	CAppProfile::GetDirPath(path, MAX_PATH_NTFS);
	PathAppend(path, _T("commands.ini"));
	in->mCommandFilePath = path;

}

CommandRepository::~CommandRepository()
{
}

// コマンドプロバイダ登録
void CommandRepository::RegisterProvider(
	CommandProvider* provider
)
{
	CSingleLock sl(&in->mCS, TRUE);

	provider->AddRef();
	in->mProviders.push_back(provider);
	std::sort(in->mProviders.begin(), in->mProviders.end(),
	          [](const CommandProvider* l, const CommandProvider* r) { return l->GetOrder() < r->GetOrder(); });
}

// コマンドを登録
int CommandRepository::RegisterCommand(Command* command)
{
	CSingleLock sl(&in->mCS, TRUE);
	in->mCommands.Register(command);
	return 0;
}

// コマンドの登録を解除
int CommandRepository::UnregisterCommand(Command* command)
{
	CSingleLock sl(&in->mCS, TRUE);
	in->mCommands.Unregister(command);
	return 0;
}

// 順位の更新
void CommandRepository::AddRank(Command* command, int number)
{
	CSingleLock sl(&in->mCS, TRUE);
	in->mRanking.Add(command->GetName(), number);
}

CommandRepository* CommandRepository::GetInstance()
{
	static CommandRepository inst;
	return &inst;
}

BOOL CommandRepository::Load()
{
	CSingleLock sl(&in->mCS, TRUE);
	// 既存の内容を破棄
	in->mCommands.Clear();

	// キーワード比較処理の生成
	in->ReloadPatternObject();

	auto tmpProviders = in->mProviders;
	sl.Unlock();

	// 設定ファイルを読み、コマンド一覧を登録する
	CommandFile commandFile;
	commandFile.SetFilePath(in->mCommandFilePath);
	commandFile.Load();

	for (auto& provider : tmpProviders) {
		provider->LoadCommands(&commandFile);
	}

	in->mRanking.Load();

	return TRUE;
}


class ScopeEdit
{
public:
	ScopeEdit(bool& flag) : mFlagPtr(&flag)
 	{
		*mFlagPtr = true;
	}
	~ScopeEdit()
	{
		*mFlagPtr = false;
	}
	bool* mFlagPtr;
};


/**
 *  新規キーワード作成
 *  @param paramr パラメータ
 */
int CommandRepository::NewCommandDialog(const CommandParameter* param)
{
	if (in->mIsNewDialog) {
		// 編集操作中の再入はしない
		return -1;
	}
	ScopeEdit scopeEdit(in->mIsNewDialog);

	CommandProvider* selectedProvider = nullptr;

	// 種類選択ダイアログを表示
	CString typeStr;
	if (param == nullptr || param->GetNamedParam(_T("TYPE"), &typeStr) == false) {
		SelectCommandTypeDialog dlgSelect;

		for (auto& provider : in->mProviders) {
			if (provider->IsPrivate()) {
				continue;
			}

			dlgSelect.AddType(provider->GetDisplayName(), provider->GetDescription(), (LPARAM)provider);
		}

		if (dlgSelect.DoModal() != IDOK) {
			return 0;
		}
		selectedProvider = (CommandProvider*)dlgSelect.GetSelectedItem();
	}
	else {
		for (auto& provider : in->mProviders) {
			if (typeStr != provider->GetName()) {
				continue;
			}
			selectedProvider = provider;
			break;
		}
	}

	if (selectedProvider == nullptr) {
		return 0;
	}

	if (selectedProvider->NewDialog(param) == false) {
		return 1;
	}

	// コマンド設定ファイルに保存
	in->SaveCommands();

	return 0;

	
}

/**
 *  既存キーワードの編集
 */
int CommandRepository::EditCommandDialog(const CString& cmdName)
{
	if (in->mIsEditDialog) {
		// 編集操作中の再入はしない
		return 0;
	}
	ScopeEdit scopeEdit(in->mIsEditDialog);

	auto cmdAbs = in->mCommands.Get(cmdName);
	if (cmdAbs == nullptr) {
		return 1;
	}

	int ret =cmdAbs->EditDialog();
	cmdAbs->Release();

	if (ret != 0) {
		return 2;
	}

	// コマンドファイルに保存
	in->SaveCommands();

	return 0;
}

/**
 * 指定したコマンド名は組み込みコマンドか?
 */
bool CommandRepository::IsBuiltinName(const CString& cmdName)
{
	CSingleLock sl(&in->mCS, TRUE);

	auto* cmd = in->mCommands.Get(cmdName);
	if (cmd == nullptr) {
		return false;
	}
	bool isEditable = cmd->IsEditable();

	cmd->Release();

	return isEditable == false;
}

/**
 *  キーワードマネージャーの表示
 */
int CommandRepository::ManagerDialog()
{
	if (in->mIsManagerDialog) {
		// 編集操作中の再入はしない
		return 0;
	}
	ScopeEdit scopeEdit(in->mIsManagerDialog);

	// キャンセル時用のバックアップ
	CommandMap commandsBkup(in->mCommands);

	KeywordManagerDialog dlg;
	if (dlg.DoModal() != IDOK) {

		// OKではないので結果を反映しない(バックアップした内容に戻す)
		in->mCommands.Swap(commandsBkup);
	}
	else {
		// ファイルに保存
	in->SaveCommands();
	}
	return 0;
}


// まとめて登録ダイアログの表示
int CommandRepository::RegisterCommandFromFiles(
	const std::vector<CString>& files
)
{
	if (in->mIsRegisteFromFileDialog) {
		// 編集操作中の再入はしない
		return 0;
	}

	ScopeEdit scopeEdit(in->mIsRegisteFromFileDialog);

	if (files.empty()) {
		// ファイルパスの要素数が0の場合はなにもしない
		return 0;
	}

	if (files.size() > 1) {
		// 複数の場合はまとめて選択ダイアログ
		SelectFilesDialog dlg;
		dlg.SetFiles(files);

		if (dlg.DoModal() != IDOK) {
			return 0;
		}

		std::vector<CString> filesToRegister;
		dlg.GetCheckedFiles(filesToRegister);

		if (filesToRegister.empty()) {
			return 0;
		}

		// ダイアログで選択されたファイルを一括登録
		for (const auto& filePath : filesToRegister) {

			CString name(PathFindFileName(filePath));
			PathRemoveExtension(name.GetBuffer(name.GetLength()));
			name.ReleaseBuffer();
			
			if (name.IsEmpty()) {
				// .xxx というファイル名の場合にnameが空文字になるのを回避する
				name = PathFindFileName(filePath);
			}

			// パスとして使えるが、ShellExecCommandのコマンド名として許可しない文字をカットする
			ShellExecCommand::SanitizeName(name);

			CString suffix;
			
			for (int i = 1;; ++i) {
				auto cmd = QueryAsWholeMatch(name + suffix, false);
				if (cmd == nullptr) {
					break;
				}
				cmd->Release();
				suffix.Format(_T("(%d)"), i);
			}

			if (suffix.IsEmpty() == FALSE) {
				name = name + suffix;
			}

			// ダイアログで入力された内容に基づき、コマンドを新規作成する
			auto newCmd = std::make_unique<ShellExecCommand>();
			newCmd->SetName(name);

			ShellExecCommand::ATTRIBUTE normalAttr;
			normalAttr.mPath = filePath;
			newCmd->SetAttribute(normalAttr);
			in->mCommands.Register(newCmd.release());
		}

		AfxMessageBox(_T("登録しました"));
	}
	else {
		// 登録ダイアログを表示

		CString filePath = files[0];

		CString name(PathFindFileName(filePath));
		PathRemoveExtension(name.GetBuffer(name.GetLength()));
		name.ReleaseBuffer();

		CommandParameter param;
		param.SetNamedParamString(_T("TYPE"), _T("ShellExecuteCommand"));
		param.SetNamedParamString(_T("COMMAND"), name);
		param.SetNamedParamString(_T("PATH"), filePath);

		return NewCommandDialog(&param);
	}
	return 0;
}

/**
 *  コマンドの削除
 */
bool CommandRepository::DeleteCommand(const CString& cmdName)
{
	CSingleLock sl(&in->mCS, TRUE);

	in->mRanking.Delete(cmdName);

	return in->mCommands.Unregister(cmdName);
}

void CommandRepository::EnumCommands(std::vector<soyokaze::core::Command*>& enumCommands)
{
	CSingleLock sl(&in->mCS, TRUE);
	in->mCommands.Enumerate(enumCommands);
}

void
CommandRepository::Query(
	const CommandParameter& param,
	std::vector<soyokaze::core::Command*>& items
)
{
	if (in->mIsQuering) {
		// 再入はしない
		return ;
	}

	ScopeEdit scopeQuery(in->mIsQuering);

	for (auto& command : items) {
		command->Release();
	}
	items.clear();

	// パラメータが空の場合は検索しない
	if (param.IsEmpty()) {
		return;
	}

	CSingleLock sl(&in->mCS, TRUE);

	// 入力文字列をを設定
	in->mPattern->SetParam(param);

	CommandMap::CommandQueryItemList matchedItems;

	in->mCommands.Query(in->mPattern.get(), matchedItems);
	  // Note: ここで+1した参照カウントは CommandRepository::Query 呼び出し元で-1する必要あり

	// コマンドプロバイダーから一時的なコマンドを取得する
	for (auto& provider : in->mProviders) {
		provider->QueryAdhocCommands(in->mPattern.get(), matchedItems);
	}

	// 一致レベルに基づきソート
	const CommandRanking* rankPtr = &in->mRanking;

	std::sort(matchedItems.begin(), matchedItems.end(),
		[rankPtr](const CommandMap::CommandQueryItem& l, const CommandMap::CommandQueryItem& r) {
			if (r.mMatchLevel < l.mMatchLevel) { return true; }
			if (r.mMatchLevel > l.mMatchLevel) { return false; }

			// 一致レベルが同じ場合は優先順位による判断を行う
			int priorityL = rankPtr->Get(l.mCommand->GetName());
			int priorityR = rankPtr->Get(r.mCommand->GetName());
			return priorityR < priorityL;
	});

	items.reserve(matchedItems.size());
	for (auto& item : matchedItems) {
		item.mCommand->AddRef();
		items.push_back(item.mCommand.get());
	}
}

soyokaze::core::Command*
CommandRepository::QueryAsWholeMatch(
	const CString& strQueryStr,
	bool isIncludeAdhocCommand
)
{
	CSingleLock sl(&in->mCS, TRUE);

	if (in->mIsQuering) {
		// 再入はしない
		return nullptr;
	}

	// パラメータが空の場合は検索しない
	if (strQueryStr.IsEmpty()) {
		return nullptr;
	}

	ScopeEdit scopeQuery(in->mIsQuering);


	WholeMatchPattern pat(strQueryStr);

	auto command = in->mCommands.FindOne(&pat);
	if (command != nullptr) {
		return command;
	}

	if (isIncludeAdhocCommand == false) {
		return nullptr;
	}

	// コマンドプロバイダーから一時的なコマンドを取得する
	soyokaze::CommandQueryItemList matchedItems;
	for (auto& provider : in->mProviders) {
		provider->QueryAdhocCommands(&pat, matchedItems);

		if (matchedItems.empty()) {
			continue;
		}

		// 完全一致のものを探す
		for (auto& item : matchedItems) {
			if (item.mMatchLevel != Pattern::WholeMatch) {
				continue;
			}
			command = item.mCommand.get();
			command->AddRef();

			return command;
		}
	}

	return nullptr;
}

bool CommandRepository::HasCommand(const CString& strQueryStr)
{
	CSingleLock sl(&in->mCS, TRUE);
	return  in->mCommands.Has(strQueryStr);
}

bool CommandRepository::IsValidAsName(const CString& strQueryStr)
{
	return strQueryStr.FindOneOf(_T(" !\"\\/*;:[]|&<>,")) == -1;
}

void CommandRepository::OnAppFirstBoot()
{
	// 初回起動であることをコマンドプロバイダに通知
	for (auto& provider : in->mProviders) {
		provider->OnFirstBoot();
	}

	// コマンド設定ファイルを保存
	in->SaveCommands();

}

void CommandRepository::OnAppPreferenceUpdated()
{
	// アプリ設定変更の影響を受ける項目の再登録
	in->ReloadPatternObject();
}

void CommandRepository::OnAppExit()
{
	// 優先度情報をファイルに保存する
	in->mRanking.Save();

	for (auto& provider : in->mProviders) {
		provider->Release();
	}
	in->mProviders.clear();

	AppPreference::Get()->UnregisterListener(this);
}

}
}

