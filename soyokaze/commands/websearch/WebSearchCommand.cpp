#include "pch.h"
#include "WebSearchCommand.h"
#include "commands/websearch/WebSearchCommandParam.h"
#include "commands/websearch/WebSearchSettingDialog.h"
#include "commands/common/ExpandFunctions.h"
#include "core/CommandRepository.h"
#include "utility/LastErrorString.h"
#include "AppPreference.h"
#include "CommandFile.h"
#include "IconLoader.h"
#include "resource.h"
#include <assert.h>

using namespace soyokaze::commands::common;

namespace soyokaze {
namespace commands {
namespace websearch {

struct WebSearchCommand::PImpl
{
	CommandParam mParam;

	bool mIsShortcut = false;
	CString mSearchWord;

	CString mErrorMsg;
	uint32_t mRefCount = 1;
};

CString WebSearchCommand::GetType() { return _T("WebSearch"); }

WebSearchCommand::WebSearchCommand() : in(std::make_unique<PImpl>())
{
}

WebSearchCommand::~WebSearchCommand()
{
}

CString WebSearchCommand::GetName()
{
	if (in->mIsShortcut == false) {
		return in->mParam.mName;
	}
	else {
		return in->mSearchWord;
	}
}

CString WebSearchCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString WebSearchCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMANDNAME_WEBSEARCH);
	return TEXT_TYPE;
}

BOOL WebSearchCommand::Execute()
{
	Parameter param;
	return Execute(param);
}

BOOL WebSearchCommand::Execute(const Parameter& param)
{
	CString path = in->mParam.mURL;

	std::vector<CString> args;
	if (in->mIsShortcut) {
		// 先頭のキーワード(本来はコマンド名)も含める
		Parameter::GetParameters(param.GetWholeString(), args);
	}
	else {
		param.GetParameters(args);
	}

	ExpandArguments(path, args);
	ExpandClipboard(path);

	in->mErrorMsg.Empty();

	SHELLEXECUTEINFO si = {};
	si.cbSize = sizeof(si);
	si.nShow = SW_SHOW;
	si.fMask = SEE_MASK_NOCLOSEPROCESS;
	si.lpFile = path;

	BOOL bRun = ShellExecuteEx(&si);
	if (bRun == FALSE) {
		LastErrorString errStr(GetLastError());
		in->mErrorMsg = (LPCTSTR)errStr;
		return FALSE;
	}

	CloseHandle(si.hProcess);

	return TRUE;
}

CString WebSearchCommand::GetErrorString()
{
	return in->mErrorMsg;
}

HICON WebSearchCommand::GetIcon()
{
	return IconLoader::Get()->LoadWebIcon();
}

int WebSearchCommand::Match(Pattern* pattern)
{
	in->mIsShortcut = false;
	in->mSearchWord.Empty();

	// キーワード名にマッチする場合
	int matchLevel = pattern->Match(GetName());
	if (matchLevel != Pattern::Mismatch) {
		return matchLevel;
	}

	// 完全一致検索の場合は検索ワード補完をしない
	if (pattern->shouldWholeMatch()) {
		return Pattern::Mismatch;
	}

	// 入力欄に入力された文字列を検索ワードとみなし候補に表示するか?
	if (in->mParam.IsEnableShortcutSearch() == false) {
		return Pattern::Mismatch;
	}

	in->mIsShortcut = true;
	in->mSearchWord = pattern->GetWholeString();

	return Pattern::PartialMatch;
}

bool WebSearchCommand::IsEditable()
{
	return true;
}

int WebSearchCommand::EditDialog(const Parameter*)
{
	SettingDialog dlg;
	dlg.SetIcon(GetIcon());

	auto param = in->mParam;

	dlg.SetParam(param);
	if (dlg.DoModal() != IDOK) {
		return 0;
	}

	auto cmdNew = std::make_unique<WebSearchCommand>();

	param = dlg.GetParam();
	cmdNew->in->mParam = param;

	// 名前が変わっている可能性があるため、いったん削除して再登録する
	auto cmdRepo = soyokaze::core::CommandRepository::GetInstance();
	cmdRepo->UnregisterCommand(this);
	cmdRepo->RegisterCommand(cmdNew.release());

	return 0;
}

soyokaze::core::Command*
WebSearchCommand::Clone()
{
	auto clonedCmd = std::make_unique<WebSearchCommand>();

	clonedCmd->in->mParam = in->mParam;

	return clonedCmd.release();
}

bool WebSearchCommand::Save(CommandFile* cmdFile)
{
	ASSERT(cmdFile);

	auto entry = cmdFile->NewEntry(GetName());
	cmdFile->Set(entry, _T("Type"), GetType());

	cmdFile->Set(entry, _T("description"), GetDescription());

	cmdFile->Set(entry, _T("URL"), in->mParam.mURL);
	cmdFile->Set(entry, _T("IsEnableShortcut"), in->mParam.mIsEnableShortcut);

	return true;
}

uint32_t WebSearchCommand::AddRef()
{
	return ++in->mRefCount;
}

uint32_t WebSearchCommand::Release()
{
	uint32_t n = --in->mRefCount;
	if (n == 0) {
		delete this;
	}
	return n;
}

bool WebSearchCommand::NewDialog(
	const Parameter* param,
	std::unique_ptr<WebSearchCommand>& newCmd
)
{
	// パラメータ指定には対応していない
	// param;

	// 新規作成ダイアログを表示
	SettingDialog dlg;
	dlg.SetIcon(IconLoader::Get()->LoadWebIcon());
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto commandParam = dlg.GetParam();
	auto command = std::make_unique<WebSearchCommand>();
	command->in->mParam = commandParam;

	newCmd = std::move(command);

	return true;
}

bool WebSearchCommand::LoadFrom(
	CommandFile* cmdFile,
 	void* e,
	std::unique_ptr<WebSearchCommand>& newCmd
)
{
	CommandFile::Entry* entry = (CommandFile::Entry*)e;
	CString typeStr = cmdFile->Get(entry, _T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != WebSearchCommand::GetType()) {
		return false;
	}

	CString name = cmdFile->GetName(entry);
	CString descriptionStr = cmdFile->Get(entry, _T("description"), _T(""));
	CString url = cmdFile->Get(entry, _T("URL"), _T(""));
	bool isEnableShortcut = cmdFile->Get(entry, _T("IsEnableShortcut"), false);


	auto command = std::make_unique<WebSearchCommand>();

	command->in->mParam.mName = name;
	command->in->mParam.mDescription = descriptionStr;
	command->in->mParam.mURL = url;
	command->in->mParam.mIsEnableShortcut = isEnableShortcut;

	newCmd = std::move(command);

	return true;
}

} // end of namespace websearch
} // end of namespace commands
} // end of namespace soyokaze

