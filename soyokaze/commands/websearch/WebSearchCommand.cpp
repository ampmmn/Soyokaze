#include "pch.h"
#include "WebSearchCommand.h"
#include "commands/websearch/WebSearchCommandParam.h"
#include "commands/websearch/WebSearchSettingDialog.h"
#include "commands/common/SubProcess.h"
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

bool WebSearchCommand::IsEnableShortcut() const
{
	return in->mParam.IsEnableShortcutSearch();
}

WebSearchCommand* WebSearchCommand::CloneAsAdhocCommand(CString& searchWord)
{
	auto newCmd = new WebSearchCommand();
	newCmd->in->mParam = in->mParam;
	newCmd->in->mIsShortcut = true;
	newCmd->in->mSearchWord = searchWord;

	return newCmd;
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

CString WebSearchCommand::GetGuideString()
{
	return _T("Enter:検索を実行");
}

CString WebSearchCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_COMMANDNAME_WEBSEARCH);
	return TEXT_TYPE;
}

BOOL WebSearchCommand::Execute(const Parameter& param_)
{
	in->mErrorMsg.Empty();

	Parameter param(param_);

	std::vector<CString> args;
	if (in->mIsShortcut) {
		// 先頭のキーワード(本来はコマンド名)も含める
		param.SetParamString(param_.GetWholeString());
	}

	SubProcess exec(param);

	SubProcess::ProcessPtr process;
	if (exec.Run(in->mParam.mURL, process) == FALSE) {
		in->mErrorMsg = process->GetErrorMessage();
		return FALSE;
	}
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
	if (in->mIsShortcut == false) {
		// キーワード名にマッチする場合
		int matchLevel = pattern->Match(in->mParam.mName);
		if (matchLevel != Pattern::Mismatch) {
			in->mIsShortcut = false;
			in->mSearchWord.Empty();
			return matchLevel;
		}
		return Pattern::Mismatch;
	}
	else {
		// キーワード補完による一時的なコマンドとしてふるまう場合
		return Pattern::PartialMatch;
	}
}

bool WebSearchCommand::IsEditable()
{
	if (in->mIsShortcut == false) {
		return true;
	}
	else {
		return false;
	}
}

int WebSearchCommand::EditDialog(const Parameter*)
{
	if (in->mIsShortcut) {
		return 0;
	}

	SettingDialog dlg;
	dlg.SetIcon(GetIcon());

	auto param = in->mParam;

	dlg.SetParam(param);
	if (dlg.DoModal() != IDOK) {
		return 0;
	}

	param = dlg.GetParam();

	// 名前が変わっている可能性があるため、いったん削除して再登録する

	AddRef();
	// UnregisterCommandのときに参照カウント-1されるので、削除を防ぐために+1しておく
	// RegisterCommandの際は呼び出し側が参照カウントを上げる

	auto cmdRepo = soyokaze::core::CommandRepository::GetInstance();
	cmdRepo->UnregisterCommand(this);

	in->mParam = param;
	cmdRepo->RegisterCommand(this);

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

