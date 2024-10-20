#include "pch.h"
#include "framework.h"
#include "ShellExecCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/shellexecute/ShellExecCommandEditor.h"
#include "commands/shellexecute/ArgumentDialog.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/common/SubProcess.h"
#include "commands/core/CommandRepository.h"
#include "hotkey/CommandHotKeyManager.h"
#include "utility/LastErrorString.h"
#include "hotkey/CommandHotKeyMappings.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "SharedHwnd.h"
#include "icon/IconLoader.h"
#include "resource.h"
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using ExecuteHistory = launcherapp::commands::common::ExecuteHistory;
using CommandParameterBuilder = launcherapp::core::CommandParameterBuilder;
using CommandRepository = launcherapp::core::CommandRepository;

namespace launcherapp {
namespace commands {
namespace shellexecute {


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct ShellExecCommand::PImpl
{
	PImpl() :
		mIcon(nullptr)
	{
	}
	~PImpl()
	{
	}

	ATTRIBUTE& GetNormalAttr() { return mParam.mNormalAttr; }
	ATTRIBUTE& GetNoParamAttr() { return mParam.mNoParamAttr; }

	void SelectAttribute(const std::vector<CString>& args,ATTRIBUTE& attr);

	CommandParam mParam;

	CString mErrMsg;
	HICON mIcon;
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

CString ShellExecCommand::GetGuideString()
{
	const auto& attr = in->GetNormalAttr();
	if (attr.mPath.Find(_T("http"))==0) {
		return _T("Enter:ブラウザで開く");
	}
	else {
		CString guideStr(_T("Enter:実行"));

		if (PathFileExists(attr.mPath)) {
			guideStr += _T(" Ctrl-Enter:フォルダを開く");

			CString ext(PathFindExtension(attr.mPath));
			if (ext.CompareNoCase(_T(".exe")) == 0 || ext.CompareNoCase(_T(".bat")) == 0) {
				guideStr += _T(" Ctrl-Shift-Enter:管理者権限で実行");
			}
		}

		return guideStr;
	}
}

CString ShellExecCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_NORMALCOMMAND);
	return TEXT_TYPE;
}

BOOL ShellExecCommand::Execute(Parameter* param_)
{
	auto param = param_->Clone();

	if (param->HasParameter() == false && in->mParam.mIsShowArgDialog) {
		// 実行時引数がなく、かつ、引数無しの場合に追加入力を促す設定の場合はダイアログを表示する
		SharedHwnd hwnd;
		ArgumentDialog dlg(GetName(), CWnd::FromHandle(hwnd.GetHwnd()));
		if (dlg.DoModal() != IDOK) {
			return TRUE;
		}
		param->SetParameterString(dlg.GetArguments());
	}



	// 実行時引数が与えられた場合は履歴に登録しておく
	if (param->HasParameter()) {
		ExecuteHistory::GetInstance()->Add(_T("history"), param->GetWholeString());
	}

	in->mErrMsg.Empty();

	int paramCount = param->GetParamCount();
	std::vector<CString> args;
	args.reserve(paramCount);
	for (int i = 0; i < paramCount; ++i) {
		args.push_back(param->GetParam(i));
	}

	// パラメータあり/なしで、mNormalAttr/mNoParamAttrを切り替える
	ATTRIBUTE attr;
	in->SelectAttribute(args, attr);
		
	SubProcess exec(param);
	exec.SetShowType(attr.GetShowType());
	exec.SetWorkDirectory(attr.mDir);
	if (in->mParam.mIsRunAsAdmin) {
		exec.SetRunAsAdmin();
	}

	SubProcess::ProcessPtr process;
	if (exec.Run(attr.mPath, attr.mParam, process) == FALSE) {
		in->mErrMsg = (LPCTSTR)process->GetErrorMessage();
		param->Release();
		return FALSE;
	}

	// もしwaitするようにするのであればここで待つ
	auto namedParam = GetCommandNamedParameter(param);
	if (namedParam->GetNamedParamBool(_T("WAIT"))) {
		const int WAIT_LIMIT = 30 * 1000; // 30 seconds.
		process->Wait(WAIT_LIMIT);
	}
	param->Release();

	return TRUE;
}

CString ShellExecCommand::GetErrorString()
{
	return in->mErrMsg;
}

void ShellExecCommand::SetPath(const CString& path)
{
	in->mParam.mNormalAttr.mPath = path;
}

void ShellExecCommand::SetArgument(const CString& arg)
{
	in->mParam.mNormalAttr.mParam = arg;
}

void ShellExecCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
}


HICON ShellExecCommand::GetIcon()
{
	if (in->mParam.mIconData.empty()) {
		CString path = in->GetNormalAttr().mPath;
		ExpandMacros(path);

		return IconLoader::Get()->LoadIconFromPath(path);
	}
	else {
		if (in->mIcon == nullptr) {
			in->mIcon = IconLoader::Get()->LoadIconFromStream(in->mParam.mIconData);
			// mIconの解放はIconLoaderが行うので、ここでは行わない
		}
		return in->mIcon;
	}
}

int ShellExecCommand::Match(Pattern* pattern)
{
	int level = pattern->Match(GetName());
	if (level != Pattern::Mismatch) {
		return level;
	}

	// 名前でヒットしなかった場合、必要に応じて説明欄の文字列でもマッチングを行う
	if (in->mParam.mIsUseDescriptionForMatching == FALSE) {
		return level;
	}
	return pattern->Match(GetDescription());
}

bool ShellExecCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool ShellExecCommand::IsPriorityRankEnabled()
{
	return true;
}

launcherapp::core::Command*
ShellExecCommand::Clone()
{
	auto clonedObj = std::make_unique<ShellExecCommand>();
	clonedObj->in->mParam = in->mParam;
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

	auto newCmd = std::make_unique<ShellExecCommand>();
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
		auto cmd = cmdRepos->QueryAsWholeMatch(name + suffix, false);
		if (cmd == nullptr) {
			break;
		}
		cmd->Release();
		// 既存の場合は末尾に数字を付与
		suffix.Format(_T("(%d)"), i);
	}

	if (suffix.IsEmpty() == FALSE) {
		name = name + suffix;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = std::make_unique<ShellExecCommand>();
	newCmd->in->mParam.mName =name;

	ATTRIBUTE& normalAttr = newCmd->in->mParam.mNormalAttr;
	normalAttr.mPath = filePath;

	cmdRepos->RegisterCommand(newCmd.release(), false);

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

	auto command = std::make_unique<ShellExecCommand>();
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
	return in->mParam.Load(entry);
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

	in->mParam = cmdEditor->GetParam();

	// 設定変更によりアイコンが変わる可能性があるためクリアする
	in->mIcon = nullptr;

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
	auto newCmd = std::make_unique<ShellExecCommand>();
	newCmd->SetParam(paramNew);

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}


}
}
}

