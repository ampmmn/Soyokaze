#include "pch.h"
#include "framework.h"
#include "RegExpCommand.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/common/SubProcess.h"
#include "commands/regexp/RegExpCommandEditDialog.h"
#include "commands/core/CommandRepository.h"
#include "utility/LastErrorString.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace regexp {

using CommandRepository = launcherapp::core::CommandRepository;

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
		mIcon(nullptr)
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

	std::vector<uint8_t> mIconData;

	HICON mIcon;

	CString mErrMsg;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString RegExpCommand::GetType() { return _T("RegExp"); }

RegExpCommand::RegExpCommand() : in(std::make_unique<PImpl>())
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

CString RegExpCommand::GetGuideString()
{
	return _T("Enter:開く");
}

CString RegExpCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_REGEXPCOMMAND);
	return TEXT_TYPE;
}

BOOL RegExpCommand::Execute(const Parameter& param)
{
	in->mErrMsg.Empty();

	ExecuteHistory::GetInstance()->Add(_T("history"), param.GetWholeString());

	ATTRIBUTE attr = in->mNormalAttr;

	CString path;
	CString paramStr;
	try {
		const CString& wholeText = param.GetWholeString();

		tstring paramStr_ = std::regex_replace((tstring)wholeText, in->mRegex, (tstring)attr.mParam);
		paramStr = paramStr_.c_str();

		tstring path_ = std::regex_replace((tstring)wholeText, in->mRegex, (tstring)attr.mPath);
		path = path_.c_str();
	}
	catch(std::regex_error&) {
		return FALSE;
	}

	SubProcess::ProcessPtr process;

	SubProcess exec(param);
	if (in->mRunAs) {
		exec.SetRunAsAdmin();
	}
	exec.SetShowType(attr.mShowType);
	exec.SetWorkDirectory(attr.mDir);

	if (exec.Run(path, paramStr, process) == false) {
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
	if (in->mIconData.empty()) {
		CString path = in->mNormalAttr.mPath;
		ExpandEnv(path);

		return IconLoader::Get()->LoadIconFromPath(path);
	}
	else {
		if (in->mIcon == nullptr) {
			in->mIcon = IconLoader::Get()->LoadIconFromStream(in->mIconData);
			// mIconの解放はIconLoaderが行うので、ここでは行わない
		}
		return in->mIcon;
	}
}

int RegExpCommand::Match(Pattern* pattern)
{
	// パターンが指定されている場合はパターンによる正規表現マッチングを優先する
	if (in->mPatternStr.IsEmpty() == FALSE) {
		if (std::regex_match((tstring)pattern->GetWholeString(), in->mRegex)) {
			return Pattern::WholeMatch;
		}
	}
	return Pattern::Mismatch;
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
	dlg.mIconData = in->mIconData;

	if (dlg.DoModal() != IDOK) {
		return 1;
	}

	auto cmdNew = std::make_unique<RegExpCommand>();

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
	cmdNew->in->mIconData = dlg.mIconData;

	// 名前が変わっている可能性があるため、いったん削除して再登録する
	auto cmdRepo = CommandRepository::GetInstance();
	cmdRepo->UnregisterCommand(this);
	cmdRepo->RegisterCommand(cmdNew.release());

	return 0;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool RegExpCommand::IsPriorityRankEnabled()
{
	return true;
}

void RegExpCommand::GetAttribute(ATTRIBUTE& attr)
{
	attr = in->mNormalAttr;
}

int RegExpCommand::GetRunAs()
{
	return in->mRunAs;
}

launcherapp::core::Command*
RegExpCommand::Clone()
{
	auto clonedObj = std::make_unique<RegExpCommand>();

	clonedObj->in->mName = in->mName;
	clonedObj->in->mDescription = in->mDescription;
	clonedObj->in->mRunAs = in->mRunAs;
	clonedObj->in->mPatternStr = in->mPatternStr;
	clonedObj->in->mRegex = in->mRegex;
	clonedObj->in->mNormalAttr = in->mNormalAttr;

	return clonedObj.release();
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

	cmdFile->Set(entry, _T("IconData"), in->mIconData);

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

	cmdFile->Get(entry, _T("IconData"), in->mIconData);

	return true;
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
	auto newCmd = std::make_unique<RegExpCommand>();
	newCmd->in->mName = dlg.mName;
	newCmd->in->mDescription = dlg.mDescription;
	newCmd->in->mRunAs = dlg.mIsRunAsAdmin;
	newCmd->in->mIconData = dlg.mIconData;
	newCmd->SetMatchPattern(dlg.mPatternStr);

	RegExpCommand::ATTRIBUTE attr;
	attr.mPath = dlg.mPath;
	attr.mParam = dlg.mParameter;
	attr.mDir = dlg.mDir;
	attr.mShowType = dlg.GetShowType();
	newCmd->SetAttribute(attr);

	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());

	return true;

}

}
}
}

