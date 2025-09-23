#include "pch.h"
#include "framework.h"
#include "RegExpCommand.h"
#include "commands/regexp/RegExpExecutionTarget.h"
#include "core/IFIDDefine.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/common/ExecutablePath.h"
#include "commands/regexp/RegExpCommandEditor.h"
#include "commands/core/CommandRepository.h"
#include "actions/builtin/ExecuteAction.h"
#include "utility/LastErrorString.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "icon/CommandIcon.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;
using ExecuteAction = launcherapp::actions::builtin::ExecuteAction;

namespace launcherapp {
namespace commands {
namespace regexp {

using CommandRepository = launcherapp::core::CommandRepository;
using CommandIcon = launcherapp::icon::CommandIcon;


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct RegExpCommand::PImpl
{
	bool GetRegex(tregex& regexObject);
	CommandParam mParam;
	tregex mRegex;

	CommandIcon mIcon;

	int mMatchLevel{Pattern::Mismatch};
};

bool RegExpCommand::PImpl::GetRegex(tregex& regexObject)
{
	try {
		if (mMatchLevel == Pattern::WholeMatch) {
			regexObject = mRegex;
			return true;
		}
		if (mMatchLevel == Pattern::FrontMatch) {
			regexObject = tregex(fmt::format(_T("{}.*$"), (LPCTSTR)mParam.mPatternStr));
			return true;
		}
		if (mMatchLevel == Pattern::PartialMatch) {
			regexObject = tregex(fmt::format(_T("^.*{}.*$"), (LPCTSTR)mParam.mPatternStr));
			return true;
		}
		return false;
	}
	catch(std::regex_error&) {
		spdlog::error("invalid regex pattern.");
		return false;
	}
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString RegExpCommand::GetType() { return _T("RegExp"); }

CString RegExpCommand::TypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_REGEXPCOMMAND);
	return TEXT_TYPE;
}

RegExpCommand::RegExpCommand() : in(std::make_unique<PImpl>())
{
}

RegExpCommand::~RegExpCommand()
{
}

CString RegExpCommand::GetName()
{
	return in->mParam.mName;
}


CString RegExpCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString RegExpCommand::GetGuideString()
{
	return _T("⏎:開く");
}

CString RegExpCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

bool RegExpCommand::CanExecute(String* reasonMsg)
{
	const ATTRIBUTE& attr = in->mParam.mNormalAttr;
	ExecutablePath path(attr.mPath);
	if (path.IsExecutable() == false) {
		if (reasonMsg) {
			*reasonMsg = "！リンク切れ！";
		}
		return false;
	}
	return true;
}


bool RegExpCommand::GetAction(uint32_t modifierFlags, Action** action)
{
	if (modifierFlags == 0) {
		auto a = new ExecuteAction(new RegExpExecutionTarget(in->mMatchLevel, in->mParam));
		if (in->mParam.mRunAs != 0) {
			a->SetRunAsAdmin();
		}
		*action = a;
		return true;
	}
	return false;

}

void RegExpCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;

	if (param.mPatternStr.IsEmpty()) {
		in->mRegex = tregex();
		return;
	}

	try {
		tregex regex((LPCTSTR)param.mPatternStr);
		in->mRegex.swap(regex);
	}
	catch(std::regex_error&) {
		spdlog::error("invalid regex pattern.");
	}
}

HICON RegExpCommand::GetIcon()
{
	if (in->mIcon.IsNull()) {
		// アイコンのリロード
		if (in->mParam.mIconData.empty()) {
			CString path = in->mParam.mNormalAttr.mPath;
			ExpandMacros(path);
			in->mIcon.LoadFromPath(path);
		}
		else {
			in->mIcon.LoadFromStream(in->mParam.mIconData);
		}
	}
	return in->mIcon;
}

int RegExpCommand::Match(Pattern* pattern)
{
	tstring str = (tstring)pattern->GetWholeString();

	tsmatch match_result;
	if (std::regex_match(str, in->mRegex)) {
		// regex_matchで一致するなら完全一致
		in->mMatchLevel = Pattern::WholeMatch;
	}
	else if (std::regex_search(str, match_result, in->mRegex)) {
		if (match_result.position() == 0) {
			in->mMatchLevel = Pattern::FrontMatch;
		}
		else {
			in->mMatchLevel = Pattern::PartialMatch;
		}
	}
	else {
		in->mMatchLevel = Pattern::Mismatch;
	}
	return in->mMatchLevel;
}

bool RegExpCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	UNREFERENCED_PARAMETER(attr);

	return false;
}

launcherapp::core::Command*
RegExpCommand::Clone()
{
	auto clonedObj = make_refptr<RegExpCommand>();
	clonedObj->SetParam(in->mParam);
	return clonedObj.release();
}

bool RegExpCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());

	entry->Set(_T("description"), GetDescription());
	entry->Set(_T("runas"), in->mParam.mRunAs);
	entry->Set(_T("matchpattern"), in->mParam.mPatternStr);

	ATTRIBUTE& normalAttr = in->mParam.mNormalAttr;
	entry->Set(_T("path"), normalAttr.mPath);
	entry->Set(_T("dir"), normalAttr.mDir);
	entry->Set(_T("parameter"), normalAttr.mParam);
	entry->Set(_T("show"), normalAttr.mShowType);

	entry->SetBytes(_T("IconData"), (const uint8_t*)in->mParam.mIconData.data(), in->mParam.mIconData.size());

	return true;
}

bool RegExpCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != RegExpCommand::GetType()) {
		return false;
	}

	CommandParam param;
	param.mName = entry->GetName();
	param.mDescription = entry->Get(_T("description"), _T(""));
	param.mRunAs = entry->Get(_T("runas"), 0);

	ATTRIBUTE& attr = param.mNormalAttr;
	attr.mPath = entry->Get(_T("path"), _T(""));
	attr.mDir = entry->Get(_T("dir"), _T(""));
	attr.mParam = entry->Get(_T("parameter"), _T(""));
	attr.mShowType = entry->Get(_T("show"), attr.mShowType);

	param.mPatternStr = entry->Get(_T("matchpattern"), _T("")); 

	size_t len = entry->GetBytesLength(_T("IconData"));
	if (len != CommandEntryIF::NO_ENTRY) {
		param.mIconData.resize(len);
		entry->GetBytes(_T("IconData"), (uint8_t*)param.mIconData.data(), len);
	}

	SetParam(param);

	return true;
}

bool RegExpCommand::NewDialog(Parameter* param)
{
	CString value;
	CommandParam paramTmp;

	if (GetNamedParamString(param, _T("COMMAND"), value)) {
		paramTmp.mName = value;
	}
	if (GetNamedParamString(param, _T("PATH"), value)) {
		paramTmp.mNormalAttr.mPath = value;
	}
	if (GetNamedParamString(param, _T("DESCRIPTION"), value)) {
		paramTmp.mDescription = value;
	}
	if (GetNamedParamString(param, _T("ARGUMENT"), value)) {
		paramTmp.mNormalAttr.mParam = value;
	}

	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	cmdEditor->SetParam(paramTmp);
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<RegExpCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());

	return true;

}

// コマンドを編集するためのダイアログを作成/取得する
bool RegExpCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
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
bool RegExpCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_REGEXPCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	in->mIcon.Reset();
	SetParam(cmdEditor->GetParam());

	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool RegExpCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_REGEXPCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	auto paramNew = cmdEditor->GetParam();

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<RegExpCommand>();
	newCmd->SetParam(paramNew);

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

}
}
}

