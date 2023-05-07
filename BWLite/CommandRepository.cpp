#include "pch.h"
#include "framework.h"
#include "CommandRepository.h"
#include "CommandMap.h"
#include "AboutDlg.h"
#include "AppProfile.h"
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
#include "commands/ExecutableFileCommand.h"
#include "CommandEditDialog.h"
#include "KeywordManagerDialog.h"
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

struct CommandRepository::PImpl
{
	PImpl() : mPattern(nullptr), mIsNewDialog(false), mIsEditDialog(false), mIsManagerDialog(false)
	{
	}
	~PImpl()
	{
		delete mPattern;
	}

	void ReloadPatternObject()
	{
		delete mPattern;

		int matchLevel = AppPreference::Get()->mMatchLevel;
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
};


CommandRepository::CommandRepository() : in(new PImpl)
{
	AppPreference::Get()->RegisterListener(this);
}

CommandRepository::~CommandRepository()
{
	AppPreference::Get()->UnregisterListener(this);
	delete in;
}



BOOL CommandRepository::Load()
{
	// 既存の内容を破棄
	in->mCommands.Clear();
	in->mBuiltinCommands.Clear();

	// キーワード比較処理の生成
	in->ReloadPatternObject();

	// ビルトインコマンドの登録
	in->mBuiltinCommands.Register(new NewCommand(this));
	in->mBuiltinCommands.Register(new EditCommand(this));
	in->mBuiltinCommands.Register(new ReloadCommand(this));
	in->mBuiltinCommands.Register(new ManagerCommand(this));
	in->mBuiltinCommands.Register(new ExitCommand());
	in->mBuiltinCommands.Register(new VersionCommand());
	in->mBuiltinCommands.Register(new UserDirCommand());
	in->mBuiltinCommands.Register(new MainDirCommand());
	in->mBuiltinCommands.Register(new SettingCommand());

	// 設定ファイルを読み、コマンド一覧を登録する
	TCHAR path[32768];
	CAppProfile::GetDirPath(path, 32768);
	PathAppend(path, _T("commands.ini"));
	in->mCommandLoader.SetFilePath(path);

	std::vector<Command*> commands;
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
 *  @param cmdNamePtr コマンドに紐づけるパス(nullの場合はパスを空欄にする)
 */
int CommandRepository::NewCommandDialog(
	const CString* cmdNamePtr,
	const CString* pathPtr
)
{
	if (in->mIsNewDialog) {
		// 編集操作中の再入はしない
		return 0;
	}
	ScopeEdit scopeEdit(in->mIsNewDialog);

	// 新規作成ダイアログを表示
	CommandEditDialog dlg(this);

	if (cmdNamePtr) {
		dlg.SetName(*cmdNamePtr);
	}
	if (pathPtr) {
		dlg.SetPath(*pathPtr);
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
	std::vector<Command*> cmdsTmp;
	in->mCommandLoader.Save(in->mCommands.Enumerate(cmdsTmp));
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

	CommandEditDialog dlg(this);
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
	std::vector<Command*> cmdsTmp;
	in->mCommandLoader.Save(in->mCommands.Enumerate(cmdsTmp));

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

	KeywordManagerDialog dlg(this);

	if (dlg.DoModal() != IDOK) {

		// OKではないので結果を反映しない(バックアップした内容に戻す)
		in->mBuiltinCommands.Swap(builtinBkup);
		in->mCommands.Swap(commandsBkup);
	}
	else {
		// ファイルに保存
		std::vector<Command*> cmdsTmp;
		in->mCommandLoader.Save(in->mCommands.Enumerate(cmdsTmp));
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

void CommandRepository::EnumCommands(std::vector<Command*>& enumCommands)
{
	enumCommands.clear();
	in->mCommands.Enumerate(enumCommands);
	in->mBuiltinCommands.Enumerate(enumCommands);
}

void
CommandRepository::Query(
	const CString& strQueryStr,
	std::vector<Command*>& items
)
{
	items.clear();

	in->mPattern->SetPattern(strQueryStr);

	in->mCommands.Query(in->mPattern, items);
	in->mBuiltinCommands.Query(in->mPattern, items);

	// 1件もマッチしない場合はExecutableCommandのひかく
	if (items.empty()) {
		if (in->mExeCommand.Match(in->mPattern)) {
			items.push_back(&in->mExeCommand);
		}
	}
}

Command* CommandRepository::QueryAsWholeMatch(
	const CString& strQueryStr
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
	if (in->mExeCommand.Match(in->mPattern)) {
		return &in->mExeCommand;
	}

	return nullptr;
}

bool CommandRepository::IsValidAsName(const CString& strQueryStr)
{
	return strQueryStr.FindOneOf(_T(" !\"\\/*;:[]|&<>,.")) == -1;
}

void CommandRepository::OnAppPreferenceUpdated()
{
	// アプリ設定変更の影響を受ける項目の再登録
	in->ReloadPatternObject();
}
