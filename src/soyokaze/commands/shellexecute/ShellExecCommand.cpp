#include "pch.h"
#include "framework.h"
#include "ShellExecCommand.h"
#include "core/IFIDDefine.h"
#include "commands/shellexecute/ShellExecTarget.h"
#include "commands/core/CommandRepository.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/shellexecute/ShellExecCommandEditor.h"
#include "commands/shellexecute/ExtraActionSettings.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/common/ExecutablePath.h"
#include "commands/explorepath/ExplorePathCommand.h"
#include "actions/builtin/ExecuteAction.h"
#include "actions/builtin/OpenPathInFilerAction.h"
#include "actions/builtin/ShowPropertiesAction.h"
#include "actions/activate_window/RestoreWindowAction.h"
#include "hotkey/CommandHotKeyManager.h"
#include "utility/LastErrorString.h"
#include "utility/Path.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "matcher/PartialMatchPattern.h"
#include "SharedHwnd.h"
#include "icon/IconLoader.h"
#include "icon/CommandIcon.h"
#include "resource.h"
#include <tuple>
#include <vector>
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using ExecuteHistory = launcherapp::commands::common::ExecuteHistory;
using NamedParameter = launcherapp::actions::core::NamedParameter;
using CommandRepository = launcherapp::core::CommandRepository;
using CommandIcon = launcherapp::icon::CommandIcon;
using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;
using OpenPathInFilerAction = launcherapp::actions::builtin::OpenPathInFilerAction;
using ShowPropertiesAction = launcherapp::actions::builtin::ShowPropertiesAction;

using ExplorePathCommand = launcherapp::commands::explorepath::ExplorePathCommand;
using SelectionBehavior = launcherapp::core::SelectionBehavior;

namespace launcherapp {
namespace commands {
namespace shellexecute {


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct ShellExecCommand::PImpl
{
	ATTRIBUTE& GetNormalAttr() { return mParam.mNormalAttr; }
	ATTRIBUTE& GetNoParamAttr() { return mParam.mNoParamAttr; }

	void SelectAttribute(const std::vector<CString>& args,ATTRIBUTE& attr);

	bool IsDirectory() {
		if (mDirectoryState == -1) {
			CString path = GetNormalAttr().mPath;
			ExpandMacros(path);
			mDirectoryState = Path::IsDirectory(path) ? 1 : 0;
		}
		return mDirectoryState == 1;
	}

	CommandParam mParam;
	CommandIcon mIcon;
	int mDirectoryState=-1;
};

// パラメータの有無などでATTRIBUTEを切り替える
void
ShellExecCommand::PImpl::SelectAttribute(
	const std::vector<CString>& args,
	ATTRIBUTE& attr
)
{
	const auto& attrNormal = GetNormalAttr();
	const auto& attrNoParam = GetNoParamAttr();

	if (args.size() > 0) {
		// パラメータあり

		// mNormalAttr優先
		if (attrNormal.mPath.IsEmpty() == FALSE) {
			attr = attrNormal;
		}
		else {
			attr = attrNoParam;
		}
	}
	else {
		// パラメータなし

		// mNoParamAttr優先
		if (attrNoParam.mPath.IsEmpty() == FALSE) {
			attr = attrNoParam;
		}
		else {
			attr = attrNormal;
		}
	}
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString ShellExecCommand::GetType() { return _T("ShellExec"); }

ShellExecCommand::ShellExecCommand() : in(std::make_unique<PImpl>())
{
}

ShellExecCommand::~ShellExecCommand()
{
}

CString ShellExecCommand::GetName()
{
	return in->mParam.mName;
}


CString ShellExecCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString ShellExecCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

// コマンドを実行可能かどうか調べる
bool ShellExecCommand::CanExecute(String* reasonMsg)
{
	const auto& attr = in->GetNormalAttr();
	ExecutablePath path(attr.mPath);
	if (path.IsExecutable() == false) {
		if (reasonMsg) {
			*reasonMsg = "！リンク切れ！";
		}
		return false;
	}
	return true;
}

bool ShellExecCommand::GetAction(const HOTKEY_ATTR& hotkeyAttr, Action** action)
{
	// 条件に合致するウインドウが存在する場合はウインドウ切替を行う
	HWND hwndTarget = in->mParam.mActivateWindowParam.FindHwnd();
	bool shouldSwitchWindow = in->mParam.mActivateWindowParam.mIsEnable && IsWindow(hwndTarget);

	auto modifierFlags = hotkeyAttr.GetModifiers();
	if (modifierFlags == 0) {
		if (shouldSwitchWindow) {
			*action = new launcherapp::actions::activate_window::RestoreWindowAction(hwndTarget);
			return true;
		}
		else {
			return CreateExecuteAction(action, false);
		}
	}
	else if (modifierFlags == MOD_SHIFT && shouldSwitchWindow) {
		// 「条件に合致するウインドウが存在する場合はウインドウ切替を行う」場合でも、Shift-Enterで通常起動もできるようにする
		return CreateExecuteAction(action, false);
	}
	else if (modifierFlags == MOD_CONTROL) {
		return CreateOpenPathAction(action);
	}
	else if (modifierFlags == (MOD_CONTROL | MOD_SHIFT)) {
		return CreateExecuteAction(action, true);
	}
	else if (modifierFlags == MOD_ALT) {
		return CreateShowPropertiesAction(action);
	}
	return false;
}

bool ShellExecCommand::CreateExecuteAction(Action** action, bool isForceRunAs)
{
	if (isForceRunAs && in->mParam.IsPathURL()) {
		// URLに対して管理者権限の実行はできない
		return false;
	}

	// パスを実行する
	auto a = new ExecuteAction(new ShellExecTarget(in->mParam));
	a->SetHistoryPolicy(ExecuteAction::HISTORY_HASPARAMONLY);

	// 管理者権限で実行
	if (isForceRunAs || in->mParam.mIsRunAsAdmin) {
		a->SetRunAsAdmin();
	}
	// 追加の環境変数をセットする
	for (auto& item : in->mParam.mEnviron) {
		auto value = item.second;
		ExpandMacros(value);
		a->SetAdditionalEnvironment(item.first, value);
	}

	*action = a;
	return true;
}

bool ShellExecCommand::CreateOpenPathAction(Action** action)
{
	if (in->mParam.IsPathURL()) {
		// URLに対してパスを開くはできない
		return false;
	}
	*action = new OpenPathInFilerAction(new ShellExecTarget(in->mParam));
	return true;
}

bool ShellExecCommand::CreateShowPropertiesAction(Action** action)
{
	if (in->mParam.IsPathURL()) {
		// URLに対してプロパティの表示はできない
		return false;
	}

	*action = new ShowPropertiesAction(new ShellExecTarget(in->mParam));
	return true;
}

void ShellExecCommand::SetPath(const CString& path)
{
	in->mParam.mNormalAttr.mPath = path;
}

void ShellExecCommand::SetArgument(const CString& arg)
{
	in->mParam.mNormalAttr.mParam = arg;
}

void ShellExecCommand::SetWorkDir(const CString& path)
{
	in->mParam.mNormalAttr.mDir = path;
}

void ShellExecCommand::SetShowType(int showType)
{
	in->mParam.mNormalAttr.mShowType = showType;
}

void ShellExecCommand::SetParam(const CommandParam& param)
{
	// 更新前に有効パラメータが存在し，かつ、自動実行を許可する場合は
	// 以前の名前で登録していた、履歴の除外ワードを解除する
	if (in->mParam.mName.IsEmpty() == FALSE && IsAllowAutoExecute()) {
		ExecuteHistory::GetInstance()->RemoveExcludeWord(in->mParam.mName);
	}

	// パラメータを上書き
	in->mParam = param;
	in->mDirectoryState = -1;

	// 更新後に自動実行を許可する場合は履歴の除外ワードを登録する
	// (自動実行したいコマンド名が履歴に含まれると、自動実行を阻害することがあるため)
	if (in->mParam.mName.IsEmpty() == FALSE && IsAllowAutoExecute()) {
		ExecuteHistory::GetInstance()->AddExcludeWord(in->mParam.mName);
	}
}


HICON ShellExecCommand::GetIcon()
{
	if (in->mParam.mIconData.empty()) {
		CString path = in->GetNormalAttr().mPath;
		ExpandMacros(path);
		in->mIcon.LoadFromPath(path);
	}
	else {
		if (in->mIcon.IsNull()) {
			in->mIcon.LoadFromStream(in->mParam.mIconData);
		}
	}
	return in->mIcon;
}

int ShellExecCommand::Match(Pattern* pattern)
{
	int level = pattern->Match(GetName());
	if (level != Pattern::Mismatch) {
		return level;
	}

	// 設定されたパスがフォルダの場合、末尾に\があったときは子要素を表示する
	if (in->IsDirectory()) {
		CString nameWithSep = GetName() + _T("\\");
		level = pattern->Match(nameWithSep);
		if (level != Pattern::Mismatch) {
			return level;
		}
	}

	// 名前でヒットしなかった場合、必要に応じて説明欄の文字列でもマッチングを行う
	if (in->mParam.mIsUseDescriptionForMatching) {
		return pattern->Match(GetDescription());
	}

	return Pattern::Mismatch;
}

bool ShellExecCommand::IsAllowAutoExecute()
{
	return in->mParam.mIsAllowAutoExecute;
}

bool ShellExecCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
ShellExecCommand::Clone()
{
	auto clonedObj = make_refptr<ShellExecCommand>();
	clonedObj->SetParam(in->mParam);
	return clonedObj.release();
}

bool ShellExecCommand::NewDialog(
	Parameter* param,
	ShellExecCommand** newCmdPtr
)
{
	CommandParam commandParam;
	GetNamedParamString(param, _T("COMMAND"), commandParam.mName);
	GetNamedParamString(param, _T("DESCRIPTION"), commandParam.mDescription);
	GetNamedParamString(param, _T("PATH"), commandParam.mNormalAttr.mPath);
	GetNamedParamString(param, _T("ARGUMENT"), commandParam.mNormalAttr.mParam);

	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	cmdEditor->SetParam(commandParam);
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	auto newCmd = make_refptr<ShellExecCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}
	return true;
}

bool ShellExecCommand::NewCommand(const CString& filePath)
{
	CString name(PathFindFileName(filePath));
	PathRemoveExtension(name.GetBuffer(name.GetLength()));
	name.ReleaseBuffer();

	if (name.IsEmpty()) {
		// .xxx というファイル名の場合にnameが空文字になるのを回避する
		name = PathFindFileName(filePath);
	}

	auto cmdRepos = CommandRepository::GetInstance();
	// パスとして使えるが、ShellExecCommandのコマンド名として許可しない文字をカットする
	SanitizeName(name);

	CString suffix;

	// 重複しないコマンド名を決定する
	for (int i = 1;; ++i) {
		RefPtr<launcherapp::core::Command> cmd(cmdRepos->QueryAsWholeMatch(name + suffix, false));
		if (cmd == nullptr) {
			break;
		}
		// 既存の場合は末尾に数字を付与
		suffix.Format(_T("(%d)"), i);
	}

	if (suffix.IsEmpty() == FALSE) {
		name = name + suffix;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<ShellExecCommand>();
	newCmd->in->mParam.mName =name;

	ATTRIBUTE& normalAttr = newCmd->in->mParam.mNormalAttr;
	normalAttr.mPath = filePath;

	cmdRepos->RegisterCommand(newCmd.release());

	return true;
}

bool ShellExecCommand::LoadFrom(
	CommandFile* cmdFile,
	void* e,
	ShellExecCommand** newCmdPtr
)
{
	UNREFERENCED_PARAMETER(cmdFile);
	assert(newCmdPtr);

	CommandFile::Entry* entry = (CommandFile::Entry*)e;

	auto command = make_refptr<ShellExecCommand>();
	if (command->Load(entry) == false) {
		return false;
	}

	if (newCmdPtr) {
		*newCmdPtr = command.release();
	}

	return true;
}

// ShellExecCommandのコマンド名として許可しない文字を置換する
CString& ShellExecCommand::SanitizeName(
	CString& str
)
{
	str.Replace(_T(' '), _T('_'));
	str.Replace(_T('!'), _T('_'));
	str.Replace(_T('['), _T('_'));
	str.Replace(_T(']'), _T('_'));
	return str;
}

bool ShellExecCommand::Save(CommandEntryIF* entry)
{
	entry->Set(_T("Type"), GetType());

	return in->mParam.Save(entry);
}

bool ShellExecCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != ShellExecCommand::GetType()) {
		return false;
	}

	CommandParam param;
	if (param.Load(entry) == false) {
		return false;
	}

	SetParam(param);
	return true;
}

// コマンドを編集するためのダイアログを作成/取得する
bool ShellExecCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
{
	if (editor == nullptr) {
		return false;
	}

	auto cmdEditor = new CommandEditor(CWnd::FromHandle(parent));
	cmdEditor->SetParam(in->mParam);

	*editor = cmdEditor;
	return true;
}

// ダイアログ上での編集結果をコマンドに適用する
bool ShellExecCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_SHELLEXECCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	SetParam(cmdEditor->GetParam());
	// 設定変更によりアイコンが変わる可能性があるためクリアする
	in->mIcon.Reset();

	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool ShellExecCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_SHELLEXECCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	auto paramNew = cmdEditor->GetParam();

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<ShellExecCommand>();
	newCmd->SetParam(paramNew);

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

// メニューの項目数を取得する
int ShellExecCommand::GetMenuItemCount()
{
	return in->mParam.IsPathURL() ? 1 : 4;
}

// メニューの表示名を取得する
bool ShellExecCommand::GetMenuItem(int index, Action** action)
{
	if (index < 0 || 3 < index) {
		return false;
	}

	if (index == 0) {
		return CreateExecuteAction(action, false);
	}
	else if (index == 1) {
		return CreateOpenPathAction(action);
	}
	else if (index == 2) {
		return CreateExecuteAction(action, true);
	}
	else {
		return CreateShowPropertiesAction(action);
	}
}

bool ShellExecCommand::QueryCandidates(Pattern* pattern, CommandQueryItemList& commands)
{
	if (in->IsDirectory() == false) {
		return false;
	}

	// パターンは <コマンド名>\<サブパス> に区切る
	CString wholeWord = pattern->GetWholeString();
	wholeWord.Trim();
	int n = wholeWord.Find(_T('\\'));
	auto commandPart = wholeWord.Left(n);
	auto pathPart = wholeWord.Mid(n+1);

	// コマンド名が合致していなければMismatch扱い
	if (commandPart.CompareNoCase(GetName()) != 0) {
		return false;
	}

	// サブパスを末尾の\\で区切る ( <中間パス>\<検索文字列> )
	CString intermediatePath;
	CString filePattern;
	bool found = false;
	int pathPartLen = pathPart.GetLength();
	for (int i = pathPartLen-1; i >= 0; --i) {
		auto c = pathPart[i];
		if (c == _T('\\') || c == _T('/')) {
			intermediatePath = pathPart.Left(i);
			filePattern = pathPart.Mid(i+1).Trim();
			found = true;
			break;
		}
	}

	if (found == false) {
		filePattern = pathPart;
	}

	// コマンドが持つディレクトリパスと中間パスを連結して、ディレクトリパスを生成する
	CString currentDir = in->GetNormalAttr().mPath;
	ExpandMacros(currentDir);
	if (intermediatePath.IsEmpty() == FALSE) {
		currentDir += _T('\\');
		currentDir += intermediatePath;
	}

	if (Path::IsDirectory(currentDir) == FALSE) {
		// 区切り文字の前のパスがフォルダパスではない
		return false;
	}

	// ディレクトリパス直下の要素をFindFileで列挙して、検索文字列と合致するものを探す
	RefPtr<PartialMatchPattern> patTmp(PartialMatchPattern::Create());
	patTmp->SetWholeText(filePattern);

	std::vector<std::tuple<int, ExplorePathCommand*> > fileTargets;   // ファイル要素

	CString findPattern(currentDir + _T("\\*.*"));
	CFileFind f;
	BOOL isLoop = f.FindFile(findPattern, 0);

	CString completionText(GetName());  // 補完用のテキスト
	if (intermediatePath.IsEmpty() == FALSE) {
		completionText += _T('\\');
		completionText += intermediatePath;
	}

	// フォルダ以下の要素を列挙する
	while (isLoop) {
		isLoop = f.FindNextFile();
		if (f.IsDots()) {
			continue;
		}

		CString fileName = f.GetFileName();
		int level = patTmp->Match(fileName);
		if (level == Pattern::Mismatch) {
			continue;
		}

		CString filePath = f.GetFilePath();

		auto newCmd = new ExplorePathCommand(PathFindFileName(filePath), filePath);
		newCmd->SetCompletionText(completionText+_T("\\")+fileName);
		newCmd->SetExtraActionSettings(ExtraActionSettings::Get()->GetSettings());

		if (f.IsDirectory()) {
			// ディレクトリ要素を先に表示
			commands.Add(CommandQueryItem(level, newCmd));
		}
		else {
			// ファイルは後ろに表示するため、いったんリストに入れる
			fileTargets.push_back({level, newCmd});
		}
	}
	f.Close();

	// ファイル要素を後に表示
	for (auto elem : fileTargets) {
		commands.Add(CommandQueryItem(std::get<0>(elem), std::get<1>(elem)));
	}
	return true;
}

void ShellExecCommand::ClearCache()
{
	// 特に何もしない
}

// 選択された
void ShellExecCommand::OnSelect(Command*)
{
	// 何もしない
}

// 選択解除された
void ShellExecCommand::OnUnselect(Command*) 
{
	// 何もしない
}

// 実行後のウインドウを閉じる方法
SelectionBehavior::CloseWindowPolicy
ShellExecCommand::GetCloseWindowPolicy(uint32_t modifierMask)
{
	return SelectionBehavior::CLOSEWINDOW_ASYNC;
}

// 選択時に入力欄に設定するキーワードとキャレットを設定する
bool ShellExecCommand::CompleteKeyword(CString& keyword, int& startPos, int& endPos)
{
	if (in->IsDirectory() == false) {
		return false;
	}

	keyword = GetName();		
	keyword += _T("\\");

	startPos = keyword.GetLength();
	endPos = keyword.GetLength();
	return true;
}


bool ShellExecCommand::QueryInterface(const launcherapp::core::IFID& ifid, void** cmd)
{
	if (UserCommandBase::QueryInterface(ifid, cmd)) {
		return true;
	}

	if (ifid == IFID_CONTEXTMENUSOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ContextMenuSource*)this;
		return true;
	}
	if (ifid == IFID_EXTRACANDIDATESOURCE) {
		AddRef();
		*cmd = (launcherapp::commands::core::ExtraCandidateSource*)this;
		return true;
	}
	if (ifid == IFID_SELECTIONBEHAVIOR) {
		AddRef();
		*cmd = (launcherapp::core::SelectionBehavior*)this;
		return true;
	}
	return false;
}

CString ShellExecCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_NORMALCOMMAND);
	return TEXT_TYPE;
}

}
}
}

