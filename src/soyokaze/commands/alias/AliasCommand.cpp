#include "pch.h"
#include "framework.h"
#include "AliasCommand.h"
#include "commands/core/IFIDDefine.h"
#include "commands/common/Clipboard.h"
#include "commands/alias/AliasCommandEditor.h"
#include "commands/core/CommandRepository.h"
#include "hotkey/CommandHotKeyManager.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "mainwindow/controller/MainWindowController.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp {
namespace commands {
namespace alias {

using CommandRepository = launcherapp::core::CommandRepository;

struct AliasCommand::PImpl
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

CString AliasCommand::GetType() { return _T("Alias"); }

CString AliasCommand::TypeDisplayName()
{
	static CString TEXT_TYPE(_T("エイリアスコマンド"));
	return TEXT_TYPE;
}

AliasCommand::AliasCommand() : in(std::make_unique<PImpl>())
{
}

AliasCommand::~AliasCommand()
{
}

CString AliasCommand::GetName()
{
	return in->mParam.mName;
}


CString AliasCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString AliasCommand::GetGuideString()
{
	return _T("⏎:実行");
}

CString AliasCommand::GetTypeDisplayName()
{
	return TypeDisplayName();
}

BOOL AliasCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	if (in->mParam.mIsPasteOnly) {
		mainWnd->SetText((LPCTSTR)in->mParam.mText);

		// フォーカスをメインウインドウに移す
		bool isToggle = false;
		mainWnd->ActivateWindow(isToggle);
	}
	else {
		bool isWaitSync = true;
		mainWnd->RunCommand((LPCTSTR)in->mParam.mText, isWaitSync);
	}

	return TRUE;
}

CString AliasCommand::GetErrorString()
{
	return _T("");
}


void AliasCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
}

HICON AliasCommand::GetIcon()
{
	// ToDo: アイコン設定
	return IconLoader::Get()->GetImageResIcon(-5301);
}

int AliasCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool AliasCommand::IsAllowAutoExecute()
{
	return in->mParam.mIsAllowAutoExecute;
}

bool AliasCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
AliasCommand::Clone()
{
	auto clonedObj = make_refptr<AliasCommand>();
	clonedObj->in->mParam = in->mParam;
	return clonedObj.release();
}

bool AliasCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	return in->mParam.Save(entry);
}

bool AliasCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != GetType()) {
		return false;
	}
	return in->mParam.Load(entry);
}

bool AliasCommand::NewDialog(Parameter* param)
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
	auto newCmd = make_refptr<AliasCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	CommandRepository::GetInstance()->RegisterCommand(newCmd.release());
	return true;

}

// コマンドを編集するためのダイアログを作成/取得する
bool AliasCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
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
bool AliasCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_ALIASCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	in->mParam = cmdEditor->GetParam();
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool AliasCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_ALIASCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}
	auto newCmd = make_refptr<AliasCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

}
}
}

