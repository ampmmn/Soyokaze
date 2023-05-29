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
#include "CommandFile.h"
#include "commands/NewCommand.h"
#include "commands/ShellExecCommand.h"
#include "commands/ReloadCommand.h"
#include "commands/EditCommand.h"
#include "commands/ExitCommand.h"
#include "commands/VersionCommand.h"
#include "commands/UserDirCommand.h"
#include "commands/MainDirCommand.h"
#include "commands/SettingCommand.h"
#include "commands/ManagerCommand.h"
#include "commands/RegistWinCommand.h"
#include "commands/ExecutableFileCommand.h"
#include "gui/CommandEditDialog.h"
#include "gui/KeywordManagerDialog.h"
#include "gui/SelectFilesDialog.h"
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

	// 設定ファイル(commands.ini)からの読み書きを行う
	CommandFile mCommandLoader;
	// 組み込みコマンド一覧
	CommandMap mBuiltinCommands;
	// 一般コマンド一覧
	CommandMap mCommands;
	// 環境変数PATHにあるexeを実行するためのコマンド
	ExecutableFileCommand mExeCommand;
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
}

CommandRepository::~CommandRepository()
{
	AppPreference::Get()->UnregisterListener(this);
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
	in->mBuiltinCommands.Clear();

	// キーワード比較処理の生成
	in->ReloadPatternObject();

	// ビルトインコマンドの登録
	in->mBuiltinCommands.Register(new NewCommand());
	in->mBuiltinCommands.Register(new EditCommand());
	in->mBuiltinCommands.Register(new ReloadCommand());
	in->mBuiltinCommands.Register(new ManagerCommand());
	in->mBuiltinCommands.Register(new ExitCommand());
	in->mBuiltinCommands.Register(new VersionCommand());
	in->mBuiltinCommands.Register(new UserDirCommand());
	in->mBuiltinCommands.Register(new MainDirCommand());
	in->mBuiltinCommands.Register(new SettingCommand());
	in->mBuiltinCommands.Register(new RegistWinCommand());

	// 設定ファイルを読み、コマンド一覧を登録する
	TCHAR path[32768];
	CAppProfile::GetDirPath(path, 32768);
	PathAppend(path, _T("commands.ini"));
	in->mCommandLoader.SetFilePath(path);

	std::vector<soyokaze::core::Command*> commands;
	in->mCommandLoader.Load(commands);

	for (auto cmd : commands) {
		in->mCommands.Register(cmd);
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
 *  @param cmdNamePtr 作成するコマンド名(nullの場合はコマンド名を空欄にする)
 *  @param pathPtr コマンドに紐づけるパス(nullの場合はパスを空欄にする)
 *  @param descPtr コマンドの説明(nullの場合は空欄にする)
 *  @param paramPtr パラメータ(nullの場合は空欄にする)
 */
int CommandRepository::NewCommandDialog(
	const CString* cmdNamePtr,
	const CString* pathPtr,
	const CString* descPtr,
	const CString* paramPtr
)
{
	if (in->mIsNewDialog) {
		// 編集操作中の再入はしない
		return 0;
	}
	ScopeEdit scopeEdit(in->mIsNewDialog);

	// 新規作成ダイアログを表示
	CommandEditDialog dlg;
	if (cmdNamePtr) {
		dlg.SetName(*cmdNamePtr);
	}
	if (pathPtr) {
		dlg.SetPath(*pathPtr);
	}
	if (descPtr) {
		dlg.SetDescription(*descPtr);
	}
	if (paramPtr) {
		dlg.SetParam(*paramPtr);
	}

	if (dlg.DoModal() != IDOK) {
		return 0;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto* newCmd = new ShellExecCommand();
	newCmd->SetName(dlg.mName);
	newCmd->SetDescription(dlg.mDescription);
	newCmd->SetRunAs(dlg.mIsRunAsAdmin);

	ShellExecCommand::ATTRIBUTE normalAttr;
	normalAttr.mPath = dlg.mPath;
	normalAttr.mParam = dlg.mParameter;
	normalAttr.mDir = dlg.mDir;
	normalAttr.mShowType = dlg.GetShowType();
	newCmd->SetAttribute(normalAttr);

	if (dlg.mIsUse0) {
		ShellExecCommand::ATTRIBUTE param0Attr;
		param0Attr.mPath = dlg.mPath0;
		param0Attr.mParam = dlg.mParameter0;
		param0Attr.mDir = dlg.mDir;
		param0Attr.mShowType = dlg.GetShowType();
		newCmd->SetAttributeForParam0(param0Attr);
	}
	else {
		ShellExecCommand::ATTRIBUTE param0Attr;
		newCmd->SetAttributeForParam0(param0Attr);
	}

	in->mCommands.Register(newCmd);

	// 設定ファイルに保存
	std::vector<soyokaze::core::Command*> cmdsTmp;
	in->mCommandLoader.Save(in->mCommands.Enumerate(cmdsTmp));


	// ホットキー設定を更新
	if (dlg.mHotKeyAttr.IsValid()) {

		auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
		CommandHotKeyMappings hotKeyMap;
		hotKeyManager->GetMappings(hotKeyMap);

		hotKeyMap.AddItem(dlg.mName, dlg.mHotKeyAttr);

		auto pref = AppPreference::Get();
		pref->SetCommandKeyMappings(hotKeyMap);

		pref->Save();
	}
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


	// ToDo: 後でクラス設計を見直す
	ShellExecCommand* cmd = (ShellExecCommand*)cmdAbs;

	CommandEditDialog dlg;
	dlg.SetOrgName(cmdName);

	dlg.mName = cmd->GetName();
	dlg.mDescription = cmd->GetDescription();
	dlg.mIsRunAsAdmin = cmd->GetRunAs();

	ShellExecCommand::ATTRIBUTE attr;
	cmd->GetAttribute(attr);

	dlg.mPath = attr.mPath;
	dlg.mParameter = attr.mParam;
	dlg.mDir = attr.mDir;
	dlg.SetShowType(attr.mShowType);

	cmd->GetAttributeForParam0(attr);
	dlg.mIsUse0 = (attr.mPath.IsEmpty() == FALSE);
	dlg.mPath0 = attr.mPath;
	dlg.mParameter0 = attr.mParam;

	auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
	HOTKEY_ATTR hotKeyAttr;
	bool isGlobal = false;
	if (hotKeyManager->HasKeyBinding(dlg.mName, &hotKeyAttr, &isGlobal)) {
		dlg.mHotKeyAttr = hotKeyAttr;
		dlg.mIsGlobal = isGlobal;
	}

	if (dlg.DoModal() != IDOK) {
		return TRUE;
	}

	ShellExecCommand* cmdNew = new ShellExecCommand();

	// 追加する処理
	cmdNew->SetName(dlg.mName);
	cmdNew->SetDescription(dlg.mDescription);
	cmdNew->SetRunAs(dlg.mIsRunAsAdmin);

	ShellExecCommand::ATTRIBUTE normalAttr;
	normalAttr.mPath = dlg.mPath;
	normalAttr.mParam = dlg.mParameter;
	normalAttr.mDir = dlg.mDir;
	normalAttr.mShowType = dlg.GetShowType();
	cmdNew->SetAttribute(normalAttr);

	if (dlg.mIsUse0) {
		ShellExecCommand::ATTRIBUTE param0Attr;
		param0Attr.mPath = dlg.mPath0;
		param0Attr.mParam = dlg.mParameter0;
		param0Attr.mDir = dlg.mDir;
		param0Attr.mShowType = dlg.GetShowType();
		cmdNew->SetAttributeForParam0(param0Attr);
	}
	else {
		ShellExecCommand::ATTRIBUTE param0Attr;
		cmdNew->SetAttributeForParam0(param0Attr);
	}

	// 名前が変わっている可能性があるため、いったん削除して再登録する
	in->mCommands.Unregister(cmd);
	in->mCommands.Register(cmdNew);

	// ファイルに保存
	std::vector<soyokaze::core::Command*> cmdsTmp;
	in->mCommandLoader.Save(in->mCommands.Enumerate(cmdsTmp));


	// ホットキー設定を更新
	CommandHotKeyMappings hotKeyMap;
	hotKeyManager->GetMappings(hotKeyMap);

	hotKeyMap.RemoveItem(hotKeyAttr);
	if (dlg.mHotKeyAttr.IsValid()) {
		hotKeyMap.AddItem(dlg.mName, dlg.mHotKeyAttr, dlg.mIsGlobal);
	}

	auto pref = AppPreference::Get();
	pref->SetCommandKeyMappings(hotKeyMap);

	pref->Save();

	return 0;
}

/**
 * 指定したコマンド名は組み込みコマンドか?
 */
bool CommandRepository::IsBuiltinName(const CString& cmdName)
{
	return in->mBuiltinCommands.Has(cmdName);
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
	CommandMap builtinBkup(in->mBuiltinCommands);
	CommandMap commandsBkup(in->mCommands);

	KeywordManagerDialog dlg;
	if (dlg.DoModal() != IDOK) {

		// OKではないので結果を反映しない(バックアップした内容に戻す)
		in->mBuiltinCommands.Swap(builtinBkup);
		in->mCommands.Swap(commandsBkup);
	}
	else {
		// ファイルに保存
		std::vector<soyokaze::core::Command*> cmdsTmp;
		in->mCommandLoader.Save(in->mCommands.Enumerate(cmdsTmp));
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
			for (int i = 1; QueryAsWholeMatch(name + suffix, false) != nullptr; ++i) {
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
		return NewCommandDialog(&name, &filePath);
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
	enumCommands.clear();
	in->mCommands.Enumerate(enumCommands);
	in->mBuiltinCommands.Enumerate(enumCommands);
}

void
CommandRepository::Query(
	const CString& strQueryStr,
	std::vector<soyokaze::core::Command*>& items
)
{
	items.clear();

	// 絞込みの文言を設定
	in->mPattern->SetPattern(strQueryStr);

	std::vector<CommandMap::QueryItem> matchedItems;

	in->mCommands.Query(in->mPattern, matchedItems);
	in->mBuiltinCommands.Query(in->mPattern, matchedItems);

	// 1件もマッチしない場合はExecutableCommandのひかく
	if (items.empty()) {
		int level = in->mExeCommand.Match(in->mPattern);
		if (level != Pattern::Mismatch) {
			matchedItems.push_back(CommandMap::QueryItem(level, &in->mExeCommand));
		}
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

	command = in->mBuiltinCommands.FindOne(&pat);
	if (command != nullptr) {
		return command;
	}

	// 1件もマッチしない場合はExecutableCommandのひかく
	if (isSearchPath && in->mExeCommand.Match(in->mPattern) != Pattern::Mismatch) {
		return &in->mExeCommand;
	}

	return nullptr;
}

bool CommandRepository::IsValidAsName(const CString& strQueryStr)
{
	return strQueryStr.FindOneOf(_T(" !\"\\/*;:[]|&<>,")) == -1;
}

void CommandRepository::OnAppPreferenceUpdated()
{
	// アプリ設定変更の影響を受ける項目の再登録
	in->ReloadPatternObject();
}

}
}

