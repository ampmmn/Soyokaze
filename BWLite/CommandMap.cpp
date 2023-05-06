#include "pch.h"
#include "framework.h"
#include "CommandMap.h"
#include "AboutDlg.h"
#include "AppProfile.h"
#include "ForwardMatchPattern.h"
#include "PartialMatchPattern.h"
#include "SkipMatchPattern.h"
#include "WholeMatchPattern.h"
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


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct CommandMap::PImpl
{
	PImpl() : pattern(nullptr)
	{
	}

	void RegisterBuiltinCommand(Command* cmd)
	{
		builtinCommands[cmd->GetName()] = cmd;
	}

	std::map<CString, Command*> builtinCommands;
	std::map<CString, Command*> commands;
	ExecutableFileCommand* exeCommand;
	Pattern* pattern;
};


CommandMap::CommandMap() : in(new PImpl)
{
	in->exeCommand = new ExecutableFileCommand();
}

CommandMap::~CommandMap()
{
	for(auto& item : in->commands) {
		delete item.second;
	}
	delete in->pattern;
	delete in;
}


static void TrimComment(CString& s)
{
	bool inDQ = false;

	int n = s.GetLength();
	for (int i = 0; i < n; ++i) {
		if (inDQ == false && s[i] == _T('#')) {
			s = s.Left(i);
			return;
		}

		if (inDQ == false && s[i] == _T('"')) {
			inDQ = true;
		}
		if (inDQ != true && s[i] == _T('"')) {
			inDQ = false;
		}
	}
}

BOOL CommandMap::Load()
{
	// 既存の内容を破棄
	for (auto& item : in->commands) {
		delete item.second;
	}
	in->commands.clear();
	for (auto& item : in->builtinCommands) {
		delete item.second;
	}
	in->builtinCommands.clear();

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
	in->RegisterBuiltinCommand(new NewCommand(this));
	in->RegisterBuiltinCommand(new EditCommand(this));
	in->RegisterBuiltinCommand(new ReloadCommand(this));
	in->RegisterBuiltinCommand(new ManagerCommand(this));
	in->RegisterBuiltinCommand(new ExitCommand());
	in->RegisterBuiltinCommand(new VersionCommand());
	in->RegisterBuiltinCommand(new UserDirCommand());
	in->RegisterBuiltinCommand(new MainDirCommand());
	in->RegisterBuiltinCommand(new SettingCommand());


	// 設定ファイルを読み、コマンド一覧を登録する
	TCHAR path[32768];
	CAppProfile::GetDirPath(path, 32768);
	PathAppend(path, _T("commands.ini"));

	// ファイルを読む
	CStdioFile file;
	if (file.Open(path, CFile::modeRead | CFile::shareDenyWrite) == FALSE) {
		return FALSE;
	}

	CString strCurSectionName;

	CString strCommandName;
	CString strDescription;
	ShellExecCommand::ATTRIBUTE normalAttr;
	ShellExecCommand::ATTRIBUTE noParamAttr;
	int runAs = 0;

	CString strLine;
	while(file.ReadString(strLine)) {

		TrimComment(strLine);
		strLine.Trim();

		if (strLine.IsEmpty()) {
			continue;
		}

		if (strLine[0] == _T('[')) {
			strCurSectionName = strLine.Mid(1, strLine.GetLength()-2);

 			if (strCommandName.IsEmpty() == FALSE) {
				auto command = new ShellExecCommand();
				command->SetName(strCommandName);
				command->SetDescription(strDescription);
				command->SetRunAs(runAs);

				if (normalAttr.mPath.IsEmpty() == FALSE) {
					command->SetAttribute(normalAttr);
				}
				if (noParamAttr.mPath.IsEmpty() == FALSE) {
					command->SetAttributeForParam0(noParamAttr);
				}

				in->commands[strCommandName] = command;
			}

			// 初期化
			strCommandName = strCurSectionName;
			strDescription.Empty();
			runAs = 0;

			normalAttr = ShellExecCommand::ATTRIBUTE();
			noParamAttr = ShellExecCommand::ATTRIBUTE();
			continue;
		}

		int n = strLine.Find(_T('='));
		if (n == -1) {
			continue;
		}

		CString strKey = strLine.Left(n);
		strKey.Trim();

		CString strValue = strLine.Mid(n+1);
		strValue.Trim();

		if (strKey.CompareNoCase(_T("description")) == 0) {
			strDescription = strValue;
		}
		else if (strKey.CompareNoCase(_T("runas")) == 0) {
			_stscanf_s(strValue, _T("%d"), &runAs);
		}
		else if (strKey.CompareNoCase(_T("path")) == 0) {
			normalAttr.mPath = strValue;
		}
		else if (strKey.CompareNoCase(_T("dir")) == 0) {
			normalAttr.mDir = strValue;
		}
		else if (strKey.CompareNoCase(_T("parameter")) == 0) {
			normalAttr.mParam = strValue;
		}
		else if (strKey.CompareNoCase(_T("show")) == 0) {
			_stscanf_s(strValue, _T("%d"), &normalAttr.mShowType);
		}
		else if (strKey.CompareNoCase(_T("path0")) == 0) {
			noParamAttr.mPath = strValue;
		}
		else if (strKey.CompareNoCase(_T("dir0")) == 0) {
			noParamAttr.mDir = strValue;
		}
		else if (strKey.CompareNoCase(_T("parameter0")) == 0) {
			noParamAttr.mParam = strValue;
		}
		else if (strKey.CompareNoCase(_T("show0")) == 0) {
			_stscanf_s(strValue, _T("%d"), &noParamAttr.mShowType);
		}
	}

	if (strCommandName.IsEmpty() == FALSE) {
		auto command = new ShellExecCommand();
		command->SetName(strCommandName);
		command->SetDescription(strDescription);
		command->SetRunAs(runAs);

		if (normalAttr.mPath.IsEmpty() == FALSE) {
			command->SetAttribute(normalAttr);
		}
		if (noParamAttr.mPath.IsEmpty() == FALSE) {
			command->SetAttributeForParam0(noParamAttr);
		}

		in->commands[strCommandName] = command;
	}

	return TRUE;
}

int CommandMap::NewCommandDialog(const CString* cmdNamePtr)
{
	CommandEditDialog dlg(this);

	if (cmdNamePtr) {
		dlg.SetName(*cmdNamePtr);
	}

	if (dlg.DoModal() != IDOK) {
		return 0;
	}

	// 追加する処理
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

	in->commands[dlg.mName] = newCmd;

	return 0;
}

int CommandMap::EditCommandDialog(const CString& cmdName)
{
	auto itFind = in->commands.find(cmdName);
	if (itFind == in->commands.end()) {
		return 1;
	}

	// ToDo: 後でクラス設計を見直す
	auto cmd = (ShellExecCommand*)itFind->second;

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

	// 追加する処理
	cmd->SetName(dlg.mName);
	cmd->SetDescription(dlg.mDescription);
	cmd->SetRunAs(dlg.mIsRunAsAdmin);

	ShellExecCommand::ATTRIBUTE normalAttr;
	normalAttr.mPath = dlg.mPath;
	normalAttr.mParam = dlg.mParameter;
	normalAttr.mDir = dlg.mDir;
	normalAttr.mShowType = dlg.GetShowType();
	cmd->SetAttribute(normalAttr);

	if (dlg.mIsUse0) {
		ShellExecCommand::ATTRIBUTE param0Attr;
		param0Attr.mPath = dlg.mPath0;
		param0Attr.mParam = dlg.mParameter0;
		param0Attr.mDir = dlg.mDir;
		param0Attr.mShowType = dlg.GetShowType();
		cmd->SetAttributeForParam0(param0Attr);
	}
	else {
		ShellExecCommand::ATTRIBUTE param0Attr;
		cmd->SetAttributeForParam0(param0Attr);
	}

	// 名前が変わっている可能性があるため、いったん削除して再登録する
	in->commands.erase(itFind);
	in->commands[cmd->GetName()] = cmd;

	return 0;
}

/**
 * 指定したコマンド名は組み込みコマンドか?
 */
bool CommandMap::IsBuiltinName(const CString& cmdName)
{
	return in->builtinCommands.find(cmdName) != in->builtinCommands.end();
}

int CommandMap::ManagerDialog()
{
	// キャンセル時用のバックアップ
	std::map<CString, Command*> builtinBkup;
	for (auto item : in->builtinCommands) {
		builtinBkup[item.first] = item.second->Clone();
	}
	std::map<CString, Command*> commandsBkup;
	for (auto item : in->commands) {
		commandsBkup[item.first] = item.second->Clone();
	}

	KeywordManagerDialog dlg(this);

	if (dlg.DoModal() != IDOK) {

		// OKではないので結果を反映しない(バックアップした内容に戻す)
		in->builtinCommands.swap(builtinBkup);
		in->commands.swap(commandsBkup);
	}

	// バックアップを消す
	for (auto item : builtinBkup) {
		delete item.second;
	}
	for (auto item : commandsBkup) {
		delete item.second;
	}

	return 0;
}

/**
 *  こまんどのさくじょ
 */
bool CommandMap::DeleteCommand(const CString& cmdName)
{
	auto itFind = in->commands.find(cmdName);
	if (itFind == in->commands.end()) {
		return false;
	}

	auto cmd = itFind->second;
	delete cmd;
	in->commands.erase(itFind);
	return true;
}

void CommandMap::EnumCommands(std::vector<Command*>& commands)
{
	commands.clear();
	commands.reserve(in->commands.size() + in->builtinCommands.size());
	for (auto item : in->commands) {
		commands.push_back(item.second);
	}
	for (auto item : in->builtinCommands) {
		commands.push_back(item.second);
	}
}

void
CommandMap::Query(
	const CString& strQueryStr,
	std::vector<Command*>& items
)
{
	items.clear();

	in->pattern->SetPattern(strQueryStr);

	for (auto& item : in->commands) {

		auto& command = item.second;
		if (command->Match(in->pattern) == FALSE) {
			continue;
		}
		items.push_back(command);
	}
	for (auto& item : in->builtinCommands) {

		auto& command = item.second;
		if (command->Match(in->pattern) == FALSE) {
			continue;
		}
		items.push_back(command);
	}

	// 1けんもまっちしないばあいはExecutableCommandのひかく
	if (items.empty()) {
		if (in->exeCommand->Match(in->pattern)) {
			items.push_back(in->exeCommand);
		}
	}
}

Command* CommandMap::QueryAsWholeMatch(
	const CString& strQueryStr
)
{
	WholeMatchPattern pat(strQueryStr);

	for (auto& item : in->commands) {

		auto& command = item.second;
		if (command->Match(&pat) == FALSE) {
			continue;
		}

		return item.second;
	}
	for (auto& item : in->builtinCommands) {

		auto& command = item.second;
		if (command->Match(&pat) == FALSE) {
			continue;
		}

		return item.second;
	}

	// 1件もマッチしないばあいはExecutableCommandのひかく
	if (in->exeCommand->Match(in->pattern)) {
		return in->exeCommand;
	}

	return nullptr;
}

bool CommandMap::IsValidAsName(const CString& strQueryStr)
{
	return strQueryStr.FindOneOf(_T(" !\"\\/*;:[]|&<>,.")) == -1;
}

