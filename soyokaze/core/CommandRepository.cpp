#include "pch.h"
#include "CommandRepository.h"
#include "CommandMap.h"
#include "gui/AboutDlg.h"
#include "utility/AppProfile.h"
#include "AppPreference.h"
#include "ForwardMatchPattern.h"
#include "PartialMatchPattern.h"
#include "SkipMatchPattern.h"
#include "WholeMatchPattern.h"
#include "core/CommandProviderIF.h"
#include "CommandFile.h"
#include "commands/ShellExecCommand.h"
#include "gui/KeywordManagerDialog.h"
#include "gui/SelectFilesDialog.h"
#include "gui/SelectCommandTypeDialog.h"
#include "core/CommandHotKeyManager.h"
#include "CommandHotKeyMappings.h"
#include "NamedCommandHotKeyHandler.h"
#include <vector>
#include <algorithm>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace core {

struct CommandRepository::PImpl
{
	PImpl() : 
		mPattern(nullptr), mIsNewDialog(false), mIsEditDialog(false), mIsManagerDialog(false),
		mIsRegisteFromFileDialog(false)
	{
	}
	~PImpl()
	{
		delete mPattern;
	}

	void ReloadPatternObject()
	{
		delete mPattern;

		auto pref = AppPreference::Get();
		int matchLevel = pref->GetMatchLevel();
		if (matchLevel == 2) {
			// スキップマッチング比較用
			mPattern = new SkipMatchPattern();
		}
		else if (matchLevel == 1) {
			// 部分一致比較用
			mPattern = new PartialMatchPattern();
		}
		else {
			// 前方一致比較用
			mPattern = new ForwardMatchPattern();
		}

		// コマンドのホットキー設定のリロード
		CommandHotKeyMappings hotKeyMap;
		pref->GetCommandKeyMappings(hotKeyMap);

		auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
		hotKeyManager->Clear();

		int count = hotKeyMap.GetItemCount();
		for (int i = 0; i < count; ++i) {
			auto name = hotKeyMap.GetName(i);
			HOTKEY_ATTR attr;
			hotKeyMap.GetHotKeyAttr(i, attr);

			auto handler = new NamedCommandHotKeyHandler(name);
			bool isGlobal = hotKeyMap.IsGlobal(i);

			hotKeyManager->Register(handler, attr, isGlobal);
		}
	}

	void SaveCommands()
	{
		mCommandFile.ClearEntries();

		std::vector<soyokaze::core::Command*> commands;
		for (auto command : mCommands.Enumerate(commands)) {
			command->Save(&mCommandFile);
			command->Release();
		}
		mCommandFile.Save();
	}

	std::vector<CommandProvider*> mProviders;

	// 設定ファイル(commands.ini)からの読み書きを行う
	CommandFile mCommandFile;
	// 一般コマンド一覧
	CommandMap mCommands;
	// キーワード比較用のクラス
	Pattern* mPattern;

	// 編集中フラグ
	bool mIsNewDialog;
	bool mIsEditDialog;
	bool mIsManagerDialog;
	bool mIsRegisteFromFileDialog;
};


CommandRepository::CommandRepository() : in(new PImpl)
{
	AppPreference::Get()->RegisterListener(this);

	TCHAR path[MAX_PATH_NTFS];
	CAppProfile::GetDirPath(path, MAX_PATH_NTFS);
	PathAppend(path, _T("commands.ini"));
	in->mCommandFile.SetFilePath(path);

}

CommandRepository::~CommandRepository()
{
	for (auto provider : in->mProviders) {
		provider->Release();
	}

	AppPreference::Get()->UnregisterListener(this);
}

// コマンドプロバイダ登録
void CommandRepository::RegisterProvider(
	CommandProvider* provider
)
{
	provider->AddRef();
	in->mProviders.push_back(provider);
	std::sort(in->mProviders.begin(), in->mProviders.end(),
	          [](const CommandProvider* l, const CommandProvider* r) { return l->GetOrder() < r->GetOrder(); });
}

// コマンドを登録
int CommandRepository::RegisterCommand(Command* command)
{
	in->mCommands.Register(command);
	return 0;
}

// コマンドの登録を解除
int CommandRepository::UnregisterCommand(Command* command)
{
	in->mCommands.Unregister(command);
	return 0;
}

CommandRepository* CommandRepository::GetInstance()
{
	static CommandRepository inst;
	return &inst;
}

BOOL CommandRepository::Load()
{
	// 既存の内容を破棄
	in->mCommands.Clear();

	// キーワード比較処理の生成
	in->ReloadPatternObject();

	// 設定ファイルを読み、コマンド一覧を登録する
	in->mCommandFile.Load();

	for (auto provider : in->mProviders) {
		provider->LoadCommands(&in->mCommandFile);
	}

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
			auto* newCmd = new ShellExecCommand();
			newCmd->SetName(name);

			ShellExecCommand::ATTRIBUTE normalAttr;
			normalAttr.mPath = filePath;
			newCmd->SetAttribute(normalAttr);
			in->mCommands.Register(newCmd);
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
	return in->mCommands.Unregister(cmdName);
}

void CommandRepository::EnumCommands(std::vector<soyokaze::core::Command*>& enumCommands)
{
	in->mCommands.Enumerate(enumCommands);
}

void
CommandRepository::Query(
	const CString& strQueryStr,
	std::vector<soyokaze::core::Command*>& items
)
{
	for (auto command : items) {
		command->Release();
	}
	items.clear();

	// 絞込みの文言を設定
	in->mPattern->SetPattern(strQueryStr);

	std::vector<CommandMap::QueryItem> matchedItems;

	in->mCommands.Query(in->mPattern, matchedItems);
	  // Note: ここで+1した参照カウントは CommandRepository::Query 呼び出し元で-1する必要あり

	// コマンドプロバイダーから一時的なコマンドを取得する
	for (auto provider : in->mProviders) {
		provider->QueryAdhocCommands(in->mPattern, matchedItems);
	}

	// 履歴に基づきソート
	std::sort(matchedItems.begin(), matchedItems.end(),
		[](const CommandMap::QueryItem& l, const CommandMap::QueryItem& r) {
			return r.mMatchLevel < l.mMatchLevel;
	});

	items.reserve(matchedItems.size());
	for (auto& item : matchedItems) {
		items.push_back(item.mCommand);
	}
}

soyokaze::core::Command*
CommandRepository::QueryAsWholeMatch(
	const CString& strQueryStr,
	bool isSearchPath
)
{
	WholeMatchPattern pat(strQueryStr);

	auto command = in->mCommands.FindOne(&pat);
	if (command != nullptr) {
		return command;
	}

	return nullptr;
}

bool CommandRepository::IsValidAsName(const CString& strQueryStr)
{
	return strQueryStr.FindOneOf(_T(" !\"\\/*;:[]|&<>,")) == -1;
}

void CommandRepository::OnAppFirstBoot()
{
	// 初回起動であることをコマンドプロバイダに通知
	for (auto provider : in->mProviders) {
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

}
}

