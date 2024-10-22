#include "pch.h"
#include "framework.h"
#include "SnippetCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/common/Clipboard.h"
#include "commands/snippet/SnippetCommandEditor.h"
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

struct SnippetCommand::PImpl
{
	PImpl()
	{
	}
	~PImpl()
	{
	}

	CommandParam mParam;
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
	return in->mParam.mName;
}


CString SnippetCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString SnippetCommand::GetGuideString()
{
	return _T("Enter:定型文をクリップボードにコピー");
}

CString SnippetCommand::GetTypeDisplayName()
{
	static CString TEXT_TYPE((LPCTSTR)IDS_SNIPPETCOMMAND);
	return TEXT_TYPE;
}

BOOL SnippetCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	// 定型文をマクロ置換
	CString text = in->mParam.mText;
	launcherapp::macros::core::MacroRepository::GetInstance()->Evaluate(text);

	// クリップボードにコピー
	Clipboard::Copy(text);

	return TRUE;
}

CString SnippetCommand::GetErrorString()
{
	return _T("");
}


void SnippetCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
}

HICON SnippetCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-5301);
}

int SnippetCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool SnippetCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
SnippetCommand::Clone()
{
	auto clonedObj = std::make_unique<SnippetCommand>();
	clonedObj->in->mParam = in->mParam;
	return clonedObj.release();
}

bool SnippetCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	entry->Set(_T("description"), GetDescription());
	entry->Set(_T("text"), in->mParam.mText);

	return true;
}

bool SnippetCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != GetType()) {
		return false;
	}

	in->mParam.mName = entry->GetName();
	in->mParam.mDescription = entry->Get(_T("description"), _T(""));
	in->mParam.mText = entry->Get(_T("text"), _T(""));

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr); 

	return true;
}

bool SnippetCommand::NewDialog(Parameter* param)
{
	// 新規作成ダイアログを表示
	CString value;
	CommandParam paramTmp;

	if (GetNamedParamString(param, _T("COMMAND"), value)) {
		paramTmp.mName = value;
	}
	if (GetNamedParamString(param, _T("DESCRIPTION"), value)) {
		paramTmp.mDescription = value;
	}
	if (GetNamedParamString(param, _T("TEXT"), value)) {
		paramTmp.mText = value;
	}

	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	cmdEditor->SetParam(paramTmp);
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = std::make_unique<SnippetCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release(), isReloadHotKey);
	return true;

}

// コマンドを編集するためのダイアログを作成/取得する
bool SnippetCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
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
bool SnippetCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_SNIPPETCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	in->mParam = cmdEditor->GetParam();
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool SnippetCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_SNIPPETCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}
	auto newCmd = std::make_unique<SnippetCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

}
}
}

