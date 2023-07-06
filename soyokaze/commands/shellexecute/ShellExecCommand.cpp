#include "pch.h"
#include "framework.h"
#include "ShellExecCommand.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/shellexecute/ShellExecSettingDialog.h"
#include "commands/shellexecute/ArgumentDialog.h"
#include "commands/common/ExecuteHistory.h"
#include "core/CommandRepository.h"
#include "core/CommandHotKeyManager.h"
#include "CommandHotKeyMappings.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "resource.h"
#include <assert.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace soyokaze::commands::common;
using ExecuteHistory = soyokaze::commands::common::ExecuteHistory;

namespace soyokaze {
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
		mRefCount(1)
	{
	}
	~PImpl()
	{
	}

	CommandParam mParam;

	ATTRIBUTE mNormalAttr;
	ATTRIBUTE mNoParamAttr;

	CString mErrMsg;

	// 参照カウント
	uint32_t mRefCount;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString ShellExecCommand::GetType() { return _T("ShellExec"); }

ShellExecCommand::ShellExecCommand() : in(new PImpl)
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

BOOL ShellExecCommand::Execute()
{
	Parameter param;
	return Execute(param);
}

BOOL ShellExecCommand::Execute(const Parameter& param)
{
	std::vector<CString> args;
	param.GetParameters(args);

	if (args.empty() && in->mParam.mIsShowArgDialog) {
		// 実行時引数がなく、かつ、引数無しの場合に追加入力を促す設定の場合はダイアログを表示する
		ArgumentDialog dlg(GetName());
		if (dlg.DoModal() != IDOK) {
			return TRUE;
		}
		Parameter::GetParameters(dlg.GetArguments(), args);
	}

	if (args.size() > 0) {
		ExecuteHistory::GetInstance()->Add(_T("history"), param.GetWholeString());
	}

	in->mErrMsg.Empty();

	// パラメータあり/なしで、mNormalAttr/mNoParamAttrを切り替える
	ATTRIBUTE attr;
	SelectAttribute(args, attr);

	CString path;
	CString paramStr;

	// Ctrlキーがおされて、かつ、パスが存在する場合はファイラーで表示
	bool isOpenPath = (param.GetNamedParamBool(_T("CtrlKeyPressed")) && PathFileExists(attr.mPath));
	if (isOpenPath || PathIsDirectory(attr.mPath)) {

		// 登録されたファイラーで開く
		auto pref = AppPreference::Get();

		if (pref->IsUseFiler()) {
			path = pref->GetFilerPath();
			paramStr = pref->GetFilerParam();

			// とりあえずリンク先のみをサポート
			paramStr.Replace(_T("$target"), attr.mPath);
			//
		}
		else {
			// 登録されたファイラーがない場合はエクスプローラで開く
			path = attr.mPath;
			if (PathIsDirectory(path) == FALSE) {
				PathRemoveFileSpec(path.GetBuffer(MAX_PATH_NTFS));
				path.ReleaseBuffer();
			}
			paramStr = _T("open");
		}
	}
	else {
		path = attr.mPath;
		paramStr = attr.mParam;
	}

	// argsの値を展開
	ExpandArguments(path, args);
	ExpandArguments(paramStr, args);
	//ExpandClipboard(paramStr);

	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = attr.mShowType;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = path;
	if (in->mParam.mIsRunAsAdmin && IsRunAsAdmin() == false) {
		si.lpVerb = _T("runas");
	}

	if (paramStr.IsEmpty() == FALSE) {
		si.lpParameters = paramStr;
	}
	if (attr.mDir.IsEmpty() == FALSE) {
		si.lpDirectory = attr.mDir;
	}
	BOOL bRun = ShellExecuteEx(&si);
	if (bRun == FALSE) {

		DWORD er = GetLastError();
		DWORD flags = FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS;
		void* msgBuf;
		FormatMessage(flags, NULL, er, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPTSTR)&msgBuf, 0, NULL);
		in->mErrMsg = (LPCTSTR)msgBuf;
		LocalFree(msgBuf);
		return FALSE;
	}

	// もしwaitするようにするのであればここで待つ
	if (param.GetNamedParamBool(_T("WAIT"))) {
		const int WAIT_LIMIT = 30 * 1000; // 30 seconds.
		WaitForSingleObject(si.hProcess, WAIT_LIMIT);
	}

	CloseHandle(si.hProcess);

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

	// 変数を解決
	ExpandEnv(attr.mPath);
	ExpandEnv(attr.mParam);
}

HICON ShellExecCommand::GetIcon()
{
	CString path = in->mNormalAttr.mPath;
	ExpandEnv(path);
	return IconLoader::Get()->LoadIconFromPath(path);
}

int ShellExecCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool ShellExecCommand::IsEditable()
{
	return true;
}

int ShellExecCommand::EditDialog(const Parameter* args)
{
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

	auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
	HOTKEY_ATTR hotKeyAttr;
	bool isGlobal = false;
	if (hotKeyManager->HasKeyBinding(param.mName, &hotKeyAttr, &isGlobal)) {
		param.mHotKeyAttr = hotKeyAttr;
		param.mIsGlobal = isGlobal;
	}

	SettingDialog dlg;
	dlg.SetParam(param);

	if (dlg.DoModal() != IDOK) {
		return 1;
	}

	std::unique_ptr<ShellExecCommand> cmdNew(new ShellExecCommand());

	// 追加する処理
	param = dlg.GetParam();

	cmdNew->SetName(param.mName);
	cmdNew->SetDescription(param.mDescription);
	cmdNew->SetRunAs(param.mIsRunAsAdmin);
	cmdNew->in->mParam.mIsShowArgDialog = param.mIsShowArgDialog;

	ShellExecCommand::ATTRIBUTE normalAttr;
	normalAttr.mPath = param.mPath;
	normalAttr.mParam = param.mParameter;
	normalAttr.mDir = param.mDir;
	normalAttr.mShowType = param.GetShowType();
	cmdNew->SetAttribute(normalAttr);

	if (param.mIsUse0) {
		ShellExecCommand::ATTRIBUTE param0Attr;
		param0Attr.mPath = param.mPath0;
		param0Attr.mParam = param.mParameter0;
		param0Attr.mDir = param.mDir;
		param0Attr.mShowType = param.GetShowType();
		cmdNew->SetAttributeForParam0(param0Attr);
	}
	else {
		ShellExecCommand::ATTRIBUTE param0Attr;
		cmdNew->SetAttributeForParam0(param0Attr);
	}

	// 名前が変わっている可能性があるため、いったん削除して再登録する
	auto cmdRepo = soyokaze::core::CommandRepository::GetInstance();
	cmdRepo->UnregisterCommand(this);
	cmdRepo->RegisterCommand(cmdNew.release());

	// ホットキー設定を更新
	CommandHotKeyMappings hotKeyMap;
	hotKeyManager->GetMappings(hotKeyMap);

	hotKeyMap.RemoveItem(hotKeyAttr);
	if (param.mHotKeyAttr.IsValid()) {
		hotKeyMap.AddItem(param.mName, param.mHotKeyAttr, param.mIsGlobal);
	}

	auto pref = AppPreference::Get();
	pref->SetCommandKeyMappings(hotKeyMap);

	pref->Save();


	return 0;
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

soyokaze::core::Command*
ShellExecCommand::Clone()
{
	auto clonedObj = std::unique_ptr<ShellExecCommand>(new ShellExecCommand());

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

	auto newCmd = std::unique_ptr<ShellExecCommand>(new ShellExecCommand());
	newCmd->in->mParam.mName = commandParam.mName;
	newCmd->in->mParam.mDescription = commandParam.mDescription;
	newCmd->in->mParam.mIsRunAsAdmin = (commandParam.mIsRunAsAdmin != 0);
	newCmd->in->mParam.mIsShowArgDialog =  commandParam.mIsShowArgDialog;

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

	// ホットキー設定を更新
	if (commandParam.mHotKeyAttr.IsValid()) {

		auto hotKeyManager = soyokaze::core::CommandHotKeyManager::GetInstance();
		CommandHotKeyMappings hotKeyMap;
		hotKeyManager->GetMappings(hotKeyMap);

		hotKeyMap.AddItem(commandParam.mName, commandParam.mHotKeyAttr);

		auto pref = AppPreference::Get();
		pref->SetCommandKeyMappings(hotKeyMap);

		pref->Save();
	}

	return true;
}

bool ShellExecCommand::LoadFrom(
	CommandFile* cmdFile,
	void* e,
	ShellExecCommand** newCmdPtr
)
{
	assert(newCmdPtr);

	CommandFile::Entry* entry = (CommandFile::Entry*)e;

	CString typeStr = cmdFile->Get(entry, _T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != ShellExecCommand::GetType()) {
		return false;
	}


	CString name = cmdFile->GetName(entry);
	CString descriptionStr = cmdFile->Get(entry, _T("description"), _T(""));
	int runAs = cmdFile->Get(entry, _T("runas"), 0);

	ShellExecCommand::ATTRIBUTE normalAttr;
	normalAttr.mPath = cmdFile->Get(entry, _T("path"), _T(""));
	normalAttr.mDir = cmdFile->Get(entry, _T("dir"), _T(""));
	normalAttr.mParam = cmdFile->Get(entry, _T("parameter"), _T(""));
	normalAttr.mShowType = cmdFile->Get(entry, _T("show"), normalAttr.mShowType);

	ShellExecCommand::ATTRIBUTE noParamAttr;
	noParamAttr.mPath = cmdFile->Get(entry, _T("path0"), _T(""));
	noParamAttr.mDir = cmdFile->Get(entry, _T("dir0"), _T(""));
	noParamAttr.mParam = cmdFile->Get(entry, _T("parameter0"), _T(""));
	noParamAttr.mShowType = cmdFile->Get(entry, _T("show0"), noParamAttr.mShowType);

	auto command = std::unique_ptr<ShellExecCommand>(new ShellExecCommand());
	command->in->mParam.mName = name;
	command->in->mParam.mDescription = descriptionStr;
	command->in->mParam.mIsRunAsAdmin = (runAs != 0);

	command->in->mParam.mIsShowArgDialog = cmdFile->Get(entry, _T("isShowArgInput"), 0);


	if (normalAttr.mPath.IsEmpty() == FALSE) {
		command->in->mNormalAttr = normalAttr;
	}
	if (noParamAttr.mPath.IsEmpty() == FALSE) {
		command->in->mNoParamAttr = noParamAttr;
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

bool ShellExecCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());

	cmdFile->Set(entry, _T("description"), GetDescription());
	cmdFile->Set(entry, _T("runas"), GetRunAs());
	cmdFile->Set(entry, _T("isShowArgInput"), in->mParam.mIsShowArgDialog);

	ShellExecCommand::ATTRIBUTE& normalAttr = in->mNormalAttr;
	cmdFile->Set(entry, _T("path"), normalAttr.mPath);
	cmdFile->Set(entry, _T("dir"), normalAttr.mDir);
	cmdFile->Set(entry, _T("parameter"), normalAttr.mParam);
	cmdFile->Set(entry, _T("show"), normalAttr.mShowType);

	ShellExecCommand::ATTRIBUTE& param0Attr = in->mNoParamAttr;
	cmdFile->Set(entry, _T("path0"), param0Attr.mPath);
	cmdFile->Set(entry, _T("dir0"), param0Attr.mDir);
	cmdFile->Set(entry, _T("parameter0"), param0Attr.mParam);
	cmdFile->Set(entry, _T("show0"), param0Attr.mShowType);

	return true;
}

uint32_t ShellExecCommand::AddRef()
{
	return ++in->mRefCount;
}

uint32_t ShellExecCommand::Release()
{
	auto n = --in->mRefCount;

	if (n == 0) {
		delete this;
	}
	return n;
}


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

