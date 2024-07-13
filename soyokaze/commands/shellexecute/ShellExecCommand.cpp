#include "pch.h"
#include "framework.h"
#include "ShellExecCommand.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/shellexecute/ShellExecSettingDialog.h"
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

namespace launcherapp {
namespace commands {
namespace shellexecute {


ShellExecCommand::ATTRIBUTE::ATTRIBUTE() :
	mShowType(SW_NORMAL)
{
}

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

	CommandParam mParam;

	ATTRIBUTE mNormalAttr;
	ATTRIBUTE mNoParamAttr;

	CString mErrMsg;
	HICON mIcon;
};


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
	if (in->mNormalAttr.mPath.Find(_T("http"))==0) {
		return _T("Enter:ブラウザで開く");
	}
	else {
		CString guideStr(_T("Enter:実行"));

		if (PathFileExists(in->mNormalAttr.mPath)) {
			guideStr += _T(" Ctrl-Enter:フォルダを開く");

			CString ext(PathFindExtension(in->mNormalAttr.mPath));
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

// Ctrl-Shift-Enterキー押下で実行した場合は管理者権限で実行する。
// CtrlとShiftが押されているかを判断する。
static bool IsRunAsKeyPressed(const launcherapp::core::CommandParameter& param)
{
	return param.GetNamedParamBool(_T("CtrlKeyPressed")) && param.GetNamedParamBool(_T("ShiftKeyPressed"));
}

BOOL ShellExecCommand::Execute(const Parameter& param_)
{
	Parameter param(param_);


	if (param.HasParameter() == false && in->mParam.mIsShowArgDialog) {
		// 実行時引数がなく、かつ、引数無しの場合に追加入力を促す設定の場合はダイアログを表示する
		SharedHwnd hwnd;
		ArgumentDialog dlg(GetName(), CWnd::FromHandle(hwnd.GetHwnd()));
		if (dlg.DoModal() != IDOK) {
			return TRUE;
		}
		param.SetParamString(dlg.GetArguments());
	}



	// 実行時引数が与えられた場合は履歴に登録しておく
	if (param.HasParameter()) {
		ExecuteHistory::GetInstance()->Add(_T("history"), param.GetWholeString());
	}

	in->mErrMsg.Empty();

	std::vector<CString> args;
	param.GetParameters(args);

	// パラメータあり/なしで、mNormalAttr/mNoParamAttrを切り替える
	ATTRIBUTE attr;
	SelectAttribute(args, attr);
		
	SubProcess exec(param);
	exec.SetShowType(attr.mShowType);
	exec.SetWorkDirectory(attr.mDir);
	if (in->mParam.mIsRunAsAdmin) {
		exec.SetRunAsAdmin();
	}

	SubProcess::ProcessPtr process;
	if (exec.Run(attr.mPath, attr.mParam, process) == FALSE) {
		in->mErrMsg = (LPCTSTR)process->GetErrorMessage();
		return FALSE;
	}

	// もしwaitするようにするのであればここで待つ
	if (param.GetNamedParamBool(_T("WAIT"))) {
		const int WAIT_LIMIT = 30 * 1000; // 30 seconds.
		process->Wait(WAIT_LIMIT);
	}

	return TRUE;
}

CString ShellExecCommand::GetErrorString()
{
	return in->mErrMsg;
}

ShellExecCommand& ShellExecCommand::SetName(LPCTSTR name)
{
	in->mParam.mName = name;
	return *this;
}

ShellExecCommand& ShellExecCommand::SetDescription(LPCTSTR description)
{
	in->mParam.mDescription = description;
	return *this;
}


ShellExecCommand& ShellExecCommand::SetAttribute(const ATTRIBUTE& attr)
{
	in->mNormalAttr = attr;

	return *this;
}

ShellExecCommand& ShellExecCommand::SetAttributeForParam0(const ATTRIBUTE& attr)
{
	in->mNoParamAttr = attr;
	return *this;
}

ShellExecCommand& ShellExecCommand::SetPath(LPCTSTR path)
{
	in->mNormalAttr.mPath = path;
	return *this;
}

ShellExecCommand& ShellExecCommand::SetRunAs(int runAs)
{
	in->mParam.mIsRunAsAdmin = runAs != 0;
	return *this;
}

void
ShellExecCommand::SelectAttribute(
	const std::vector<CString>& args,
	ATTRIBUTE& attr
)
{
	// パラメータの有無などでATTRIBUTEを切り替える

	if (args.size() > 0) {
		// パラメータあり

		// mNormalAttr優先
		if (in->mNormalAttr.mPath.IsEmpty() == FALSE) {
			attr = in->mNormalAttr;
		}
		else {
			attr = in->mNoParamAttr;
		}
	}
	else {
		// パラメータなし

		// mNoParamAttr優先
		if (in->mNoParamAttr.mPath.IsEmpty() == FALSE) {
			attr = in->mNoParamAttr;
		}
		else {
			attr = in->mNormalAttr;
		}
	}
}

HICON ShellExecCommand::GetIcon()
{
	if (in->mParam.mIconData.empty()) {
		CString path = in->mNormalAttr.mPath;
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

int ShellExecCommand::EditDialog(const Parameter* args)
{
	UNREFERENCED_PARAMETER(args);

	auto& param = in->mParam;

	ShellExecCommand::ATTRIBUTE attr = in->mNormalAttr;

	param.mPath = attr.mPath;
	param.mParameter = attr.mParam;
	param.mDir = attr.mDir;
	param.SetShowType(attr.mShowType);

	attr = in->mNoParamAttr;
	param.mIsUse0 = (attr.mPath.IsEmpty() == FALSE);
	param.mPath0 = attr.mPath;
	param.mParameter0 = attr.mParam;

	SettingDialog dlg;
	dlg.SetParam(param);

	if (dlg.DoModal() != IDOK) {
		return 1;
	}

	// 追加する処理
	param = dlg.GetParam();

	ShellExecCommand::ATTRIBUTE normalAttr;
	normalAttr.mPath = param.mPath;
	normalAttr.mParam = param.mParameter;
	normalAttr.mDir = param.mDir;
	normalAttr.mShowType = param.GetShowType();
	SetAttribute(normalAttr);

	if (param.mIsUse0) {
		ShellExecCommand::ATTRIBUTE param0Attr;
		param0Attr.mPath = param.mPath0;
		param0Attr.mParam = param.mParameter0;
		param0Attr.mDir = param.mDir;
		param0Attr.mShowType = param.GetShowType();
		SetAttributeForParam0(param0Attr);
	}
	else {
		ShellExecCommand::ATTRIBUTE param0Attr;
		SetAttributeForParam0(param0Attr);
	}

	// 名前が変わっている可能性があるため、いったん削除して再登録する
	auto cmdRepo = launcherapp::core::CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);


	return 0;
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

void ShellExecCommand::GetAttribute(ATTRIBUTE& attr)
{
	attr = in->mNormalAttr;
}

void ShellExecCommand::GetAttributeForParam0(ATTRIBUTE& attr)
{
	attr = in->mNoParamAttr;
}

int ShellExecCommand::GetRunAs()
{
	return in->mParam.mIsRunAsAdmin ? 1 : 0;
}

launcherapp::core::Command*
ShellExecCommand::Clone()
{
	auto clonedObj = std::make_unique<ShellExecCommand>();

	clonedObj->in->mNormalAttr = in->mNormalAttr;
	clonedObj->in->mNoParamAttr = in->mNoParamAttr;
	clonedObj->in->mParam = in->mParam;


	return clonedObj.release();
}

bool ShellExecCommand::NewDialog(
	const Parameter* param,
	ShellExecCommand** newCmdPtr
)
{
	// 新規作成ダイアログを表示
	CString value;

	CommandParam commandParam;
	if (param && param->GetNamedParam(_T("COMMAND"), &value)) {
		commandParam.mName = value;
	}
	if (param && param->GetNamedParam(_T("PATH"), &value)) {
		commandParam.mPath = value;
	}
	if (param && param->GetNamedParam(_T("DESCRIPTION"), &value)) {
		commandParam.mDescription = value;
	}
	if (param && param->GetNamedParam(_T("ARGUMENT"), &value)) {
		commandParam.mParameter = value;
	}

	SettingDialog dlg;
	dlg.SetParam(commandParam);
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	commandParam = dlg.GetParam();

	auto newCmd = std::make_unique<ShellExecCommand>();
	newCmd->in->mParam.mName = commandParam.mName;
	newCmd->in->mParam.mDescription = commandParam.mDescription;
	newCmd->in->mParam.mIsRunAsAdmin = (commandParam.mIsRunAsAdmin != 0);
	newCmd->in->mParam.mIsShowArgDialog =  commandParam.mIsShowArgDialog;
	newCmd->in->mParam.mIsUseDescriptionForMatching = commandParam.mIsUseDescriptionForMatching;
	newCmd->in->mParam.mIconData = commandParam.mIconData;

	ShellExecCommand::ATTRIBUTE normalAttr;
	normalAttr.mPath =commandParam.mPath;
	normalAttr.mParam = commandParam.mParameter;
	normalAttr.mDir = commandParam.mDir;
	normalAttr.mShowType = commandParam.GetShowType();
	newCmd->in->mNormalAttr = normalAttr;

	if (commandParam.mIsUse0) {
		ShellExecCommand::ATTRIBUTE param0Attr;
		param0Attr.mPath = commandParam.mPath0;
		param0Attr.mParam = commandParam.mParameter0;
		param0Attr.mDir = commandParam.mDir;
		param0Attr.mShowType = commandParam.GetShowType();
		newCmd->in->mNoParamAttr = param0Attr;
	}
	else {
		ShellExecCommand::ATTRIBUTE param0Attr;
		newCmd->in->mNoParamAttr = param0Attr;
	}

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

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
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());

	entry->Set(_T("description"), GetDescription());
	entry->Set(_T("runas"), GetRunAs());
	entry->Set(_T("isShowArgInput"), in->mParam.mIsShowArgDialog);

	ShellExecCommand::ATTRIBUTE& normalAttr = in->mNormalAttr;
	entry->Set(_T("path"), normalAttr.mPath);
	entry->Set(_T("dir"), normalAttr.mDir);
	entry->Set(_T("parameter"), normalAttr.mParam);
	entry->Set(_T("show"), normalAttr.mShowType);

	ShellExecCommand::ATTRIBUTE& param0Attr = in->mNoParamAttr;
	entry->Set(_T("path0"), param0Attr.mPath);
	entry->Set(_T("dir0"), param0Attr.mDir);
	entry->Set(_T("parameter0"), param0Attr.mParam);
	entry->Set(_T("show0"), param0Attr.mShowType);

	entry->Set(_T("IconData"), in->mParam.mIconData);

	return true;
}

bool ShellExecCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != ShellExecCommand::GetType()) {
		return false;
	}

	in->mParam.mName = entry->GetName();
	in->mParam.mDescription = entry->Get(_T("description"), _T(""));
	in->mParam.mIsRunAsAdmin = (entry->Get(_T("runas"), 0) != 0);

	ShellExecCommand::ATTRIBUTE normalAttr;
	normalAttr.mPath = entry->Get(_T("path"), _T(""));
	normalAttr.mDir = entry->Get(_T("dir"), _T(""));
	normalAttr.mParam = entry->Get(_T("parameter"), _T(""));
	normalAttr.mShowType = entry->Get(_T("show"), normalAttr.mShowType);
	in->mNormalAttr = normalAttr;

	ShellExecCommand::ATTRIBUTE noParamAttr;
	noParamAttr.mPath = entry->Get(_T("path0"), _T(""));
	noParamAttr.mDir = entry->Get(_T("dir0"), _T(""));
	noParamAttr.mParam = entry->Get(_T("parameter0"), _T(""));
	noParamAttr.mShowType = entry->Get(_T("show0"), noParamAttr.mShowType);
	in->mNoParamAttr = noParamAttr;

	in->mParam.mIsShowArgDialog = entry->Get(_T("isShowArgInput"), 0);

	entry->Get(_T("IconData"), in->mParam.mIconData);

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr); 

	return true;
}

// 管理者権限で実行しているか?
bool ShellExecCommand::IsRunAsAdmin()
{
	PSID grp;
	SID_IDENTIFIER_AUTHORITY authority = SECURITY_NT_AUTHORITY;
	BOOL result = AllocateAndInitializeSid(&authority, 2, SECURITY_BUILTIN_DOMAIN_RID, DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &grp);
	if (result == FALSE) {
		return false;
	}

	BOOL isMember = FALSE;
	result = CheckTokenMembership(nullptr, grp, &isMember);
	FreeSid(grp);

	return result && isMember;
}


}
}
}

