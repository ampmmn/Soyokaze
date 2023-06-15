#include "pch.h"
#include "framework.h"
#include "RegExpCommand.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/regexp/RegExpCommandEditDialog.h"
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
namespace regexp {

using CommandRepository = soyokaze::core::CommandRepository;

RegExpCommand::ATTRIBUTE::ATTRIBUTE() :
	mShowType(SW_NORMAL)
{
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct RegExpCommand::PImpl
{
	PImpl() :
		mRunAs(0),
		mRefCount(1)
	{
	}
	~PImpl()
	{
	}

	CString mName;
	CString mDescription;
	CString mPatternStr;
	tregex mRegex;
	int mRunAs;

	ATTRIBUTE mNormalAttr;

	CString mErrMsg;

	// 参照カウント
	uint32_t mRefCount;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString RegExpCommand::GetType() { return _T("RegExp"); }

RegExpCommand::RegExpCommand() : in(new PImpl)
{
}

RegExpCommand::~RegExpCommand()
{
}

CString RegExpCommand::GetName()
{
	return in->mName;
}


CString RegExpCommand::GetDescription()
{
	return in->mDescription;
}

BOOL RegExpCommand::Execute()
{
	Parameter param;
	return Execute(param);
}

BOOL RegExpCommand::Execute(const Parameter& param)
{

	in->mErrMsg.Empty();

	ATTRIBUTE attr = in->mNormalAttr;

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
		try {
			const CString& wholeText = param.GetWholeString();

			auto paramOrg = attr.mParam;
			ExpandEnv(paramOrg);
			tstring paramStr_ = std::regex_replace((tstring)wholeText, in->mRegex, (tstring)paramOrg);
			paramStr = paramStr_.c_str();

			auto pathOrg = attr.mPath;
			ExpandEnv(pathOrg);
			tstring path_ = std::regex_replace((tstring)wholeText, in->mRegex, (tstring)pathOrg);
			path = path_.c_str();
		}
		catch(std::regex_error&) {
			return FALSE;
		}
	}

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

CString RegExpCommand::GetErrorString()
{
	return in->mErrMsg;
}

RegExpCommand& RegExpCommand::SetName(LPCTSTR name)
{
	in->mName = name;
	return *this;
}

RegExpCommand& RegExpCommand::SetDescription(LPCTSTR description)
{
	in->mDescription = description;
	return *this;
}


RegExpCommand& RegExpCommand::SetAttribute(const ATTRIBUTE& attr)
{
	in->mNormalAttr = attr;

	return *this;
}

RegExpCommand& RegExpCommand::SetPath(LPCTSTR path)
{
	in->mNormalAttr.mPath = path;
	return *this;
}

RegExpCommand& RegExpCommand::SetRunAs(int runAs)
{
	in->mRunAs = runAs;
	return *this;
}

RegExpCommand& RegExpCommand::SetMatchPattern(LPCTSTR pattern)
{
	if (pattern == nullptr || _tcslen(pattern) == 0) {
		in->mPatternStr.Empty();
		in->mRegex = tregex();
		return *this;
	}

	try {
		tregex regex(pattern);

		in->mPatternStr = pattern;
		in->mRegex.swap(regex);

		return *this;
	}
	catch(std::regex_error&) {
		return *this;
	}
}

HICON RegExpCommand::GetIcon()
{
	CString path = in->mNormalAttr.mPath;
	ExpandEnv(path);
	return IconLoader::Get()->LoadIconFromPath(path);
}

int RegExpCommand::Match(Pattern* pattern)
{
	// パターンが指定されている場合はパターンによる正規表現マッチングを優先する
	if (in->mPatternStr.IsEmpty() == FALSE) {
		if (std::regex_match((tstring)pattern->GetOriginalPattern(), in->mRegex)) {
			return Pattern::WholeMatch;
		}
	}
	return Pattern::Mismatch;
}

bool RegExpCommand::IsEditable()
{
	return true;
}

int RegExpCommand::EditDialog(const Parameter* param)
{
	CommandEditDialog dlg;
	dlg.SetOrgName(in->mName);

	dlg.mName = in->mName;
	dlg.mDescription = in->mDescription;
	dlg.mIsRunAsAdmin = in->mRunAs;
	dlg.mPatternStr = in->mPatternStr;

	RegExpCommand::ATTRIBUTE attr = in->mNormalAttr;

	dlg.mPath = attr.mPath;
	dlg.mParameter = attr.mParam;
	dlg.mDir = attr.mDir;
	dlg.SetShowType(attr.mShowType);

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

	RegExpCommand* cmdNew = new RegExpCommand();

	// 追加する処理
	cmdNew->SetName(dlg.mName);
	cmdNew->SetDescription(dlg.mDescription);
	cmdNew->SetRunAs(dlg.mIsRunAsAdmin);
	cmdNew->SetMatchPattern(dlg.mPatternStr);

	RegExpCommand::ATTRIBUTE normalAttr;
	normalAttr.mPath = dlg.mPath;
	normalAttr.mParam = dlg.mParameter;
	normalAttr.mDir = dlg.mDir;
	normalAttr.mShowType = dlg.GetShowType();
	cmdNew->SetAttribute(normalAttr);

	// 名前が変わっている可能性があるため、いったん削除して再登録する
	auto cmdRepo = CommandRepository::GetInstance();
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

void RegExpCommand::GetAttribute(ATTRIBUTE& attr)
{
	attr = in->mNormalAttr;
}

int RegExpCommand::GetRunAs()
{
	return in->mRunAs;
}

soyokaze::core::Command*
RegExpCommand::Clone()
{
	auto clonedObj = new RegExpCommand();

	clonedObj->in->mName = in->mName;
	clonedObj->in->mDescription = in->mDescription;
	clonedObj->in->mRunAs = in->mRunAs;
	clonedObj->in->mPatternStr = in->mPatternStr;
	clonedObj->in->mRegex = in->mRegex;
	clonedObj->in->mNormalAttr = in->mNormalAttr;

	return clonedObj;
}

bool RegExpCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());

	cmdFile->Set(entry, _T("description"), GetDescription());
	cmdFile->Set(entry, _T("runas"), GetRunAs());
	cmdFile->Set(entry, _T("matchpattern"), in->mPatternStr);

	RegExpCommand::ATTRIBUTE& normalAttr = in->mNormalAttr;
	cmdFile->Set(entry, _T("path"), normalAttr.mPath);
	cmdFile->Set(entry, _T("dir"), normalAttr.mDir);
	cmdFile->Set(entry, _T("parameter"), normalAttr.mParam);
	cmdFile->Set(entry, _T("show"), normalAttr.mShowType);

	return true;
}

bool RegExpCommand::Load(CommandFile* cmdFile, void* entry_)
{
	auto entry = (CommandFile::Entry*)entry_;

	in->mName = cmdFile->GetName(entry);
	in->mDescription = cmdFile->Get(entry, _T("description"), _T(""));
	in->mRunAs = cmdFile->Get(entry, _T("runas"), 0);

	RegExpCommand::ATTRIBUTE& attr = in->mNormalAttr;
	attr.mPath = cmdFile->Get(entry, _T("path"), _T(""));
	attr.mDir = cmdFile->Get(entry, _T("dir"), _T(""));
	attr.mParam = cmdFile->Get(entry, _T("parameter"), _T(""));
	attr.mShowType = cmdFile->Get(entry, _T("show"), attr.mShowType);

	auto patternStr = cmdFile->Get(entry, _T("matchpattern"), _T("")); 
	SetMatchPattern(patternStr);

	return true;
}

uint32_t RegExpCommand::AddRef()
{
	return ++in->mRefCount;
}

uint32_t RegExpCommand::Release()
{
	auto n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}


bool RegExpCommand::IsRunAsAdmin()
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

bool RegExpCommand::NewDialog(const Parameter* param)
{
	// 新規作成ダイアログを表示
	CString value;

	CommandEditDialog dlg;
	if (param && param->GetNamedParam(_T("COMMAND"), &value)) {
		dlg.SetName(value);
	}
	if (param && param->GetNamedParam(_T("PATH"), &value)) {
		dlg.SetPath(value);
	}
	if (param && param->GetNamedParam(_T("DESCRIPTION"), &value)) {
		dlg.SetDescription(value);
	}
	if (param && param->GetNamedParam(_T("ARGUMENT"), &value)) {
		dlg.SetParam(value);
	}

	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = new RegExpCommand();
	newCmd->in->mName = dlg.mName;
	newCmd->in->mDescription = dlg.mDescription;
	newCmd->in->mRunAs = dlg.mIsRunAsAdmin;
	newCmd->SetMatchPattern(dlg.mPatternStr);

	RegExpCommand::ATTRIBUTE attr;
	attr.mPath = dlg.mPath;
	attr.mParam = dlg.mParameter;
	attr.mDir = dlg.mDir;
	attr.mShowType = dlg.GetShowType();
	newCmd->SetAttribute(attr);

	CommandRepository::GetInstance()->RegisterCommand(newCmd);

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

	return true;

}

}
}
}

