#include "pch.h"
#include "framework.h"
#include "ShellExecCommand.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/shellexecute/CommandEditDialog.h"
#include "core/CommandRepository.h"
#include "core/CommandHotKeyManager.h"
#include "CommandHotKeyMappings.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace soyokaze::commands::common;

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
		mRunAs(0),
		mIsUseRegExp(false),
		mRefCount(1)
	{
	}
	~PImpl()
	{
	}

	CString mName;
	CString mDescription;
	bool mIsUseRegExp;
	CString mPatternStr;
	tregex mRegex;
	int mRunAs;

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
	return in->mName;
}


CString ShellExecCommand::GetDescription()
{
	return in->mDescription;
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

	// マッチしたコマンド名
	CString matchStr = param.GetCommandString();

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
	ExpandVariable(path, _T("matchstr"), matchStr);

	if (PathIsURL(path) == FALSE && PathFileExists(path) == FALSE) {
		// ファイルがURLでなく、かつ、パスが存在しない場合はエラーにする
		CString msg((LPCTSTR)IDS_ERR_FAILTOSHELLEXECUTE);
		msg +=_T("\n\n");
		msg += CString((LPCTSTR)IDS_PATH);
		msg +=_T(":");
		msg += path;

		AfxMessageBox(msg);

		return TRUE;
	}

	ExpandArguments(paramStr, args);
	ExpandVariable(paramStr, _T("matchstr"), matchStr);

	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = attr.mShowType;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = path;
	if (in->mRunAs == 1 && IsRunAsAdmin() == false) {
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
	in->mName = name;
	return *this;
}

ShellExecCommand& ShellExecCommand::SetDescription(LPCTSTR description)
{
	in->mDescription = description;
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
	in->mRunAs = runAs;
	return *this;
}

ShellExecCommand& ShellExecCommand::SetMatchPattern(LPCTSTR pattern)
{
	if (pattern == nullptr || _tcslen(pattern) == 0) {
		in->mPatternStr.Empty();
		in->mRegex = tregex();
		in->mIsUseRegExp = false;
		return *this;
	}

	try {
		tregex regex(pattern);

		in->mPatternStr = pattern;
		in->mRegex.swap(regex);
		in->mIsUseRegExp = true;

		return *this;
	}
	catch(std::regex_error&) {
		in->mIsUseRegExp = false;
		return *this;
	}
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
	// パターンが指定されている場合はパターンによる正規表現マッチングを優先する
	if (in->mIsUseRegExp) {
		if (std::regex_match((tstring)pattern->GetOriginalPattern(), in->mRegex)) {
			return Pattern::WholeMatch;
		}
	}

	return pattern->Match(GetName());
}

bool ShellExecCommand::IsEditable()
{
	return true;
}

int ShellExecCommand::EditDialog(const Parameter* param)
{
	CommandEditDialog dlg;
	dlg.SetOrgName(in->mName);

	dlg.mName = in->mName;
	dlg.mDescription = in->mDescription;
	dlg.mIsRunAsAdmin = in->mRunAs;
	dlg.mIsUseRegExp = in->mIsUseRegExp;
	dlg.mPatternStr = in->mPatternStr;

	ShellExecCommand::ATTRIBUTE attr = in->mNormalAttr;

	dlg.mPath = attr.mPath;
	dlg.mParameter = attr.mParam;
	dlg.mDir = attr.mDir;
	dlg.SetShowType(attr.mShowType);

	attr = in->mNoParamAttr;
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
		return 1;
	}

	ShellExecCommand* cmdNew = new ShellExecCommand();

	// 追加する処理
	cmdNew->SetName(dlg.mName);
	cmdNew->SetDescription(dlg.mDescription);
	cmdNew->SetRunAs(dlg.mIsRunAsAdmin);
	cmdNew->SetMatchPattern(dlg.mIsUseRegExp ? dlg.mPatternStr : nullptr);

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
	auto cmdRepo = soyokaze::core::CommandRepository::GetInstance();
	cmdRepo->UnregisterCommand(this);
	cmdRepo->RegisterCommand(cmdNew);

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
	return in->mRunAs;
}

soyokaze::core::Command*
ShellExecCommand::Clone()
{
	auto clonedObj = new ShellExecCommand();

	clonedObj->in->mName = in->mName;
	clonedObj->in->mDescription = in->mDescription;
	clonedObj->in->mRunAs = in->mRunAs;
	clonedObj->in->mNormalAttr = in->mNormalAttr;
	clonedObj->in->mNoParamAttr = in->mNoParamAttr;

	return clonedObj;
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
	cmdFile->Set(entry, _T("useregexp"), in->mIsUseRegExp);
	cmdFile->Set(entry, _T("matchpattern"), in->mPatternStr);


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

