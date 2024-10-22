#include "pch.h"
#include "framework.h"
#include "RegExpCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/common/ExpandFunctions.h"
#include "commands/common/CommandParameterFunctions.h"
#include "commands/common/ExecuteHistory.h"
#include "commands/common/SubProcess.h"
#include "commands/regexp/RegExpCommandEditor.h"
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


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

struct RegExpCommand::PImpl
{
	CommandParam mParam;
	tregex mRegex;

	HICON mIcon = nullptr;

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
	return in->mParam.mName;
}


CString RegExpCommand::GetDescription()
{
	return in->mParam.mDescription;
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

BOOL RegExpCommand::Execute(Parameter* param)
{
	in->mErrMsg.Empty();

	ExecuteHistory::GetInstance()->Add(_T("history"), param->GetWholeString());

	const ATTRIBUTE& attr = in->mParam.mNormalAttr;

	CString path;
	CString paramStr;
	try {
		LPCTSTR wholeText = param->GetWholeString();

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
	if (in->mParam.mRunAs != 0) {
		exec.SetRunAsAdmin();
	}
	exec.SetShowType(attr.mShowType);
	exec.SetWorkDirectory(attr.mDir);

	if (exec.Run(path, paramStr, process) == false) {
		in->mErrMsg = (LPCTSTR)process->GetErrorMessage();
		return FALSE;
	}

	// もしwaitするようにするのであればここで待つ
	auto namedParam = GetCommandNamedParameter(param);
	if (namedParam->GetNamedParamBool(_T("WAIT"))) {
		const int WAIT_LIMIT = 30 * 1000; // 30 seconds.
		process->Wait(WAIT_LIMIT);
	}
	return TRUE;
}

CString RegExpCommand::GetErrorString()
{
	return in->mErrMsg;
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
	if (in->mParam.mIconData.empty()) {
		CString path = in->mParam.mNormalAttr.mPath;
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

int RegExpCommand::Match(Pattern* pattern)
{
	// パターンが指定されている場合はパターンによる正規表現マッチングを優先する
	if (in->mParam.mPatternStr.IsEmpty() == FALSE) {
		if (std::regex_match((tstring)pattern->GetWholeString(), in->mRegex)) {
			return Pattern::WholeMatch;
		}
	}
	return Pattern::Mismatch;
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

	bool isReloadHotKey = false;
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release(), isReloadHotKey);

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

	in->mIcon = nullptr;
	in->mParam = cmdEditor->GetParam();

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

