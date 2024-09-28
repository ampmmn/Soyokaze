#include "pch.h"
#include "framework.h"
#include "SnippetCommand.h"
#include "commands/common/Clipboard.h"
#include "commands/snippet/SnippetCommandEditDialog.h"
#include "commands/core/CommandRepository.h"
#include "hotkey/CommandHotKeyManager.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "macros/core/MacroRepository.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace snippet {

using CommandRepository = launcherapp::core::CommandRepository;

constexpr LPCTSTR TYPENAME = _T("SnippetCommand");

struct SnippetCommand::PImpl
{
	PImpl()
	{
	}
	~PImpl()
	{
	}

	CString mName;
	CString mDescription;
	CString mText;
	CommandHotKeyAttribute mHotKeyAttr;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString SnippetCommand::GetType() { return _T("Snippet"); }

SnippetCommand::SnippetCommand() : in(std::make_unique<PImpl>())
{
}

SnippetCommand::~SnippetCommand()
{
}

CString SnippetCommand::GetName()
{
	return in->mName;
}


CString SnippetCommand::GetDescription()
{
	return in->mDescription;
}

CString SnippetCommand::GetGuideString()
{
	return _T("Enter:定型文をクリップボードにコピー");
}

/**
 * 種別を表す文字列を取得する
 * @return 文字列
 */
CString SnippetCommand::GetTypeName()
{
	return TYPENAME;
}

CString SnippetCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_SNIPPETCOMMAND);
	return TEXT_TYPE;
}

BOOL SnippetCommand::Execute(const Parameter& param)
{
	UNREFERENCED_PARAMETER(param);

	// 定型文をマクロ置換
	CString text = in->mText;
	launcherapp::macros::core::MacroRepository::GetInstance()->Evaluate(text);

	// クリップボードにコピー
	Clipboard::Copy(text);

	return TRUE;
}

CString SnippetCommand::GetErrorString()
{
	return _T("");
}

SnippetCommand& SnippetCommand::SetName(LPCTSTR name)
{
	in->mName = name;
	return *this;
}

SnippetCommand& SnippetCommand::SetDescription(LPCTSTR description)
{
	in->mDescription = description;
	return *this;
}


SnippetCommand& SnippetCommand::SetText(const CString& text)
{
	in->mText = text;
	return *this;
}

HICON SnippetCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-5301);
}

int SnippetCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

int SnippetCommand::EditDialog(HWND parent)
{
	CommandEditDialog dlg(CWnd::FromHandle(parent));
	dlg.SetOrgName(in->mName);

	dlg.mName = in->mName;
	dlg.mDescription = in->mDescription;
	dlg.mText = in->mText;
	dlg.mHotKeyAttr = in->mHotKeyAttr;

	if (dlg.DoModal() != IDOK) {
		return 1;
	}

	// 追加する処理
	SetName(dlg.mName);
	SetDescription(dlg.mDescription);
	SetText(dlg.mText);
	in->mHotKeyAttr = dlg.mHotKeyAttr;

	auto cmdRepo = CommandRepository::GetInstance();
	cmdRepo->ReregisterCommand(this);

	return 0;
}

bool SnippetCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mHotKeyAttr;
	return true;
}

/**
 *  @brief 優先順位の重みづけを使用するか?
 *  @true true:優先順位の重みづけを使用する false:使用しない
 */
bool SnippetCommand::IsPriorityRankEnabled()
{
	return true;
}

launcherapp::core::Command*
SnippetCommand::Clone()
{
	auto clonedObj = std::make_unique<SnippetCommand>();

	clonedObj->in->mName = in->mName;
	clonedObj->in->mDescription = in->mDescription;
	clonedObj->in->mText = in->mText;

	return clonedObj.release();
}

bool SnippetCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	entry->Set(_T("description"), GetDescription());
	entry->Set(_T("text"), in->mText);

	return true;
}

bool SnippetCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != GetType()) {
		return false;
	}

	in->mName = entry->GetName();
	in->mDescription = entry->Get(_T("description"), _T(""));
	in->mText = entry->Get(_T("text"), _T(""));

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mName, &in->mHotKeyAttr); 

	return true;
}

bool SnippetCommand::NewDialog(const Parameter* param)
{
	// 新規作成ダイアログを表示
	CString value;

	CommandEditDialog dlg;
	if (param && param->GetNamedParam(_T("COMMAND"), &value)) {
		dlg.SetName(value);
	}
	if (param && param->GetNamedParam(_T("DESCRIPTION"), &value)) {
		dlg.SetDescription(value);
	}
	if (param && param->GetNamedParam(_T("TEXT"), &value)) {
		dlg.mText = value;
	}
	if (dlg.DoModal() != IDOK) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = std::make_unique<SnippetCommand>();
	newCmd->in->mName = dlg.mName;
	newCmd->in->mDescription = dlg.mDescription;
	newCmd->in->mText = dlg.mText;
	newCmd->in->mHotKeyAttr = dlg.mHotKeyAttr;

	bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release(), isReloadHotKey);
	return true;

}

}
}
}

