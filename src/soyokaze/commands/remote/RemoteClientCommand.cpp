#include "pch.h"
#include "framework.h"
#include "RemoteClientCommand.h"
#include "mainwindow/LauncherWindowEventDispatcher.h"
#include "mainwindow/LauncherWindowEventListenerIF.h"
#include "commands/remote/RemoteClientCommandParam.h"
#include "commands/remote/RemoteClient.h"
#include "commands/remote/RemoteCommandLauncher.h"
#include "commands/core/IFIDDefine.h"
#include "commands/remote/RemoteClientCommandEditor.h"
#include "commands/core/CommandRepository.h"
#include "mainwindow/controller/MainWindowController.h"
#include "hotkey/CommandHotKeyManager.h"
#include "setting/AppPreference.h"
#include "commands/core/CommandFile.h"
#include "icon/IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

using namespace launcherapp::commands::common;

namespace launcherapp { namespace commands { namespace remote {

using CommandRepository = launcherapp::core::CommandRepository;
using MainWindowController = launcherapp::mainwindow::controller::MainWindowController;

struct RemoteClientCommand::PImpl : public LauncherWindowEventListenerIF
{
	PImpl()
	{
	}
	~PImpl()
	{
	}

	void OnLockScreenOccurred() override {}
	void OnUnlockScreenOccurred() override {}
	void OnTimer() override {}
	void OnLancuherActivate() override {}
	void OnLancuherUnactivate() override {}
	void OnRunningCommand(launcherapp::core::Command*) override 
	{
		// ランチャーをローカルモードに戻す
		OnCancel();
	}
	void OnCancel() override
	{
		// ランチャーをローカルモードに戻す
		auto mainWnd = MainWindowController::GetInstance();
		mainWnd->SetLauncher(nullptr);
		LauncherWindowEventDispatcher::Get()->RemoveListener(this);
	}

	CommandParam mParam;
	RemoteClient mClient;
};


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

CString RemoteClientCommand::GetType() { return _T("RemoteClient"); }

RemoteClientCommand::RemoteClientCommand() : in(std::make_unique<PImpl>())
{
}

RemoteClientCommand::~RemoteClientCommand()
{
}

CString RemoteClientCommand::GetName()
{
	return in->mParam.mName;
}


CString RemoteClientCommand::GetDescription()
{
	return in->mParam.mDescription;
}

CString RemoteClientCommand::GetGuideString()
{
	return _T("⏎:実行");
}

CString RemoteClientCommand::GetTypeDisplayName()
{
	return _T("リモートコマンド");
}

BOOL RemoteClientCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	if (in->mClient.IsConnected() == false) {
		if (in->mClient.Connect(in->mParam) == false) {
			return FALSE;
		}
	}

	// ランチャーをリモートモードに差し替える
	auto mainWnd = MainWindowController::GetInstance();
	mainWnd->SetLauncher(new RemoteCommandLauncher(&in->mClient));

	LauncherWindowEventDispatcher::Get()->AddListener(in.get());

	// ウインドウを再表示する
	bool isShowToggle = false;
	mainWnd->ActivateWindow(isShowToggle);

	// 候補一覧を更新する
	mainWnd->UpdateCandidateRequest();

	return TRUE;
}

CString RemoteClientCommand::GetErrorString()
{
	return in->mClient.GetErrorMessage();
}


void RemoteClientCommand::SetParam(const CommandParam& param)
{
	in->mParam = param;
}

HICON RemoteClientCommand::GetIcon()
{
	return IconLoader::Get()->GetImageResIcon(-171);
}

int RemoteClientCommand::Match(Pattern* pattern)
{
	return pattern->Match(GetName());
}

bool RemoteClientCommand::GetHotKeyAttribute(CommandHotKeyAttribute& attr)
{
	attr = in->mParam.mHotKeyAttr;
	return true;
}

launcherapp::core::Command*
RemoteClientCommand::Clone()
{
	auto clonedObj = make_refptr<RemoteClientCommand>();
	clonedObj->in->mParam = in->mParam;
	return clonedObj.release();
}

bool RemoteClientCommand::Save(CommandEntryIF* entry)
{
	ASSERT(entry);

	entry->Set(_T("Type"), GetType());
	in->mParam.Save(entry);

	return true;
}

bool RemoteClientCommand::Load(CommandEntryIF* entry)
{
	ASSERT(entry);

	CString typeStr = entry->Get(_T("Type"), _T(""));
	if (typeStr.IsEmpty() == FALSE && typeStr != GetType()) {
		return false;
	}
	in->mParam.Load(entry);

	// ホットキー情報の取得
	auto hotKeyManager = launcherapp::core::CommandHotKeyManager::GetInstance();
	hotKeyManager->GetKeyBinding(in->mParam.mName, &in->mParam.mHotKeyAttr); 

	return true;
}

bool RemoteClientCommand::NewDialog(Parameter* param)
{
	// 新規作成ダイアログを表示
	CommandParam paramTmp;

	RefPtr<CommandEditor> cmdEditor(new CommandEditor());
	cmdEditor->SetParam(paramTmp);
	if (cmdEditor->DoModal() == false) {
		return false;
	}

	// ダイアログで入力された内容に基づき、コマンドを新規作成する
	auto newCmd = make_refptr<RemoteClientCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	bool isReloadHotKey = true;
	CommandRepository::GetInstance()->RegisterCommand(newCmd.release(), isReloadHotKey);
	return true;

}

// コマンドを編集するためのダイアログを作成/取得する
bool RemoteClientCommand::CreateEditor(HWND parent, launcherapp::core::CommandEditor** editor)
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
bool RemoteClientCommand::Apply(launcherapp::core::CommandEditor* editor)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_REMOTECLIENTCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}

	in->mParam = cmdEditor->GetParam();
	return true;
}

// ダイアログ上での編集結果に基づき、新しいコマンドを作成(複製)する
bool RemoteClientCommand::CreateNewInstanceFrom(launcherapp::core::CommandEditor* editor, Command** newCmdPtr)
{
	RefPtr<CommandEditor> cmdEditor;
	if (editor->QueryInterface(IFID_REMOTECLIENTCOMMANDEDITOR, (void**)&cmdEditor) == false) {
		return false;
	}
	auto newCmd = make_refptr<RemoteClientCommand>();
	newCmd->SetParam(cmdEditor->GetParam());

	if (newCmdPtr) {
		*newCmdPtr = newCmd.release();
	}

	return true;
}

}}} // end of namespace launcherapp::commands::remote

