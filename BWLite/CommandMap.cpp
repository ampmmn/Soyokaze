#include "pch.h"
#include "framework.h"
#include "CommandMap.h"
#include "AboutDlg.h"
#include "AppProfile.h"
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
#include <map>
#include <vector>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

class CommandMap
{
public:
	CommandMap()
	{
	}
	CommandMap(const CommandMap& rhs)
	{
		for (auto item : rhs.mMap) {
			mMap[item.first] = item.second->Clone();
		}
	}
	~CommandMap()
	{
		Clear();
	}

	void Clear()
	{
		for (auto item : mMap) {
			delete item.second;
		}
	}

	bool Has(const CString& name) const
	{
		return mMap.find(name) != mMap.end();
	}
	Command* Get(const CString& name)
	{
		auto itFind = mMap.find(name);
		if (itFind == mMap.end()) {
			return nullptr;
		}
		return itFind->second;
	}

	void Register(Command* cmd)
	{
		mMap[cmd->GetName()] = cmd;
	}

	bool Unregister(Command* cmd)
	{
		return Unregister(cmd->GetName());
	}

	bool Unregister(const CString& name)
	{
		auto itFind = mMap.find(name);
		if (itFind == mMap.end()) {
			return false;
		}

		delete itFind->second;
		mMap.erase(itFind);
		return true;
	}

	void Swap(CommandMap& rhs)
	{
		mMap.swap(rhs.mMap);
	}

	void Query(Pattern* pattern, std::vector<Command*>& commands)
	{
		for (auto& item : mMap) {

			auto& command = item.second;
			if (command->Match(pattern) == FALSE) {
				continue;
			}
			commands.push_back(command);
		}
	}

	// 最初に見つけた要素を返す
	Command* FindOne(Pattern* pattern)
	{
		for (auto& item : mMap) {

			auto& command = item.second;
			if (command->Match(pattern) == FALSE) {
				continue;
			}
			return item.second;
		}
		return nullptr;
	}

	std::vector<Command*>& Enumerate(std::vector<Command*>& commands)
	{
		commands.reserve(commands.size() + mMap.size());
		for (auto item : mMap) {
			commands.push_back(item.second);
		}
		return commands;
	}

protected:
	std::map<CString, Command*> mMap;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct CommandRepository::PImpl
{
	PImpl() : pattern(nullptr)
	{
	}

	CommandFile commandLoader;

	CommandMap builtinCommands;
	CommandMap commands;
	ExecutableFileCommand* exeCommand;
	Pattern* pattern;
};


CommandRepository::CommandRepository() : in(new PImpl)
{
	in->exeCommand = new ExecutableFileCommand();
}

CommandRepository::~CommandRepository()
{
	delete in->pattern;
	delete in;
}



BOOL CommandRepository::Load()
{
	// 既存の内容を破棄
	in->commands.Clear();
	in->builtinCommands.Clear();

	// キーワード比較処理の生成
	delete in->pattern;

	CAppProfile* app = CAppProfile::Get();
	int matchLevel = app->Get(_T("BWLite"), _T("MatchLevel"), 2);
	if (matchLevel == 0) {
		in->pattern = new SkipMatchPattern();
	}
	else if (matchLevel == 1) {
		in->pattern = new PartialMatchPattern();
	}
	else {
		in->pattern = new ForwardMatchPattern();
	}

	// ビルトインコマンドの登録
	in->builtinCommands.Register(new NewCommand(this));
	in->builtinCommands.Register(new EditCommand(this));
	in->builtinCommands.Register(new ReloadCommand(this));
	in->builtinCommands.Register(new ManagerCommand(this));
	in->builtinCommands.Register(new ExitCommand());
	in->builtinCommands.Register(new VersionCommand());
	in->builtinCommands.Register(new UserDirCommand());
	in->builtinCommands.Register(new MainDirCommand());
	in->builtinCommands.Register(new SettingCommand());

	// 設定ファイルを読み、コマンド一覧を登録する
	TCHAR path[32768];
	CAppProfile::GetDirPath(path, 32768);
	PathAppend(path, _T("commands.ini"));
	in->commandLoader.SetFilePath(path);

	std::vector<Command*> commands;
	in->commandLoader.Load(commands);

	for (auto cmd : commands) {
		in->commands.Register(cmd);
	}

	return TRUE;
}


/**
 *  新規キーワード作成
 *  @param cmdNamePtr 作成するコマンド名(nullの場合はコマンド名を空欄にする)
 */
int CommandRepository::NewCommandDialog(
	const CString* cmdNamePtr
)
{
	// 新規作成ダイアログを表示
	CommandEditDialog dlg(this);

	if (cmdNamePtr) {
		dlg.SetName(*cmdNamePtr);
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

	in->commands.Register(newCmd);

	// 設定ファイルに保存
	std::vector<Command*> cmdsTmp;
	in->commandLoader.Save(in->commands.Enumerate(cmdsTmp));
	return 0;
}


/**
 *  既存キーワードの編集
 */
int CommandRepository::EditCommandDialog(const CString& cmdName)
{
	auto cmdAbs = in->commands.Get(cmdName);
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
	in->commands.Unregister(cmd);
	in->commands.Register(cmdNew);

	// ファイルに保存
	std::vector<Command*> cmdsTmp;
	in->commandLoader.Save(in->commands.Enumerate(cmdsTmp));

	return 0;
}

/**
 * 指定したコマンド名は組み込みコマンドか?
 */
bool CommandRepository::IsBuiltinName(const CString& cmdName)
{
	return in->builtinCommands.Has(cmdName);
}

/**
 *  キーワードマネージャーの表示
 */
int CommandRepository::ManagerDialog()
{
	// キャンセル時用のバックアップ
	CommandMap builtinBkup(in->builtinCommands);
	CommandMap commandsBkup(in->commands);

	KeywordManagerDialog dlg(this);

	if (dlg.DoModal() != IDOK) {

		// OKではないので結果を反映しない(バックアップした内容に戻す)
		in->builtinCommands.Swap(builtinBkup);
		in->commands.Swap(commandsBkup);
	}
	else {
		// ファイルに保存
		std::vector<Command*> cmdsTmp;
		in->commandLoader.Save(in->commands.Enumerate(cmdsTmp));
	}
	return 0;
}

/**
 *  コマンドの削除
 */
bool CommandRepository::DeleteCommand(const CString& cmdName)
{
	return in->commands.Unregister(cmdName);
}

void CommandRepository::EnumCommands(std::vector<Command*>& enumCommands)
{
	enumCommands.clear();
	in->commands.Enumerate(enumCommands);
	in->builtinCommands.Enumerate(enumCommands);
}

void
CommandRepository::Query(
	const CString& strQueryStr,
	std::vector<Command*>& items
)
{
	items.clear();

	in->pattern->SetPattern(strQueryStr);

	in->commands.Query(in->pattern, items);
	in->builtinCommands.Query(in->pattern, items);

	// 1件もマッチしない場合はExecutableCommandのひかく
	if (items.empty()) {
		if (in->exeCommand->Match(in->pattern)) {
			items.push_back(in->exeCommand);
		}
	}
}

Command* CommandRepository::QueryAsWholeMatch(
	const CString& strQueryStr
)
{
	WholeMatchPattern pat(strQueryStr);

	auto command = in->commands.FindOne(&pat);
	if (command != nullptr) {
		return command;
	}

	command = in->builtinCommands.FindOne(&pat);
	if (command != nullptr) {
		return command;
	}

	// 1件もマッチしない場合はExecutableCommandのひかく
	if (in->exeCommand->Match(in->pattern)) {
		return in->exeCommand;
	}

	return nullptr;
}

bool CommandRepository::IsValidAsName(const CString& strQueryStr)
{
	return strQueryStr.FindOneOf(_T(" !\"\\/*;:[]|&<>,.")) == -1;
}

