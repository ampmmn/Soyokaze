#include "pch.h"
#include "framework.h"
#include "SettingCommand.h"
#include "settingwindow/AppSettingDialog.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "mainwindow/controller/MainWindowController.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace builtin {

CString SettingCommand::TYPE(_T("Builtin-Setting"));

// BuiltinCommandFactory経由でインスタンスを生成できるようにするための手続き
REGISTER_BUILTINCOMMAND(SettingCommand)

CString SettingCommand::GetType()
{
	return TYPE;
}

SettingCommand::SettingCommand(LPCTSTR name) :
	BuiltinCommandBase(name ? name : _T("setting"))
{
	mDescription = _T("【設定】");
	mIsExecuting = false;
}

SettingCommand::SettingCommand(const SettingCommand& rhs) :
	BuiltinCommandBase(rhs),
	mIsExecuting(rhs.mIsExecuting),
	mLastBreakCrumbs(rhs.mLastBreakCrumbs)
{
}

SettingCommand::~SettingCommand()
{
}

BOOL SettingCommand::Execute(Parameter* param)
{
	UNREFERENCED_PARAMETER(param);

	if (mIsExecuting) {
		// 既に実行中
		return TRUE;
	}

	struct scope_flag {
		scope_flag(bool& f) : mf(f) { mf= true; }
		~scope_flag() { mf = false; }
		bool& mf;
	} _local(mIsExecuting);


	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	mainWnd->RequestCallback([](LPARAM param) {
		SettingCommand* pThis = (SettingCommand*)param;
		return pThis->OnCallbackExecute();
	}, this);

	return TRUE;
}

LRESULT SettingCommand::OnCallbackExecute()
{
	AppSettingDialog dlg;

	// 前回表示していたページを復元する
	dlg.SetBreadCrumbsString(mLastBreakCrumbs);

	auto pref = AppPreference::Get();

	// 現在の設定をセット
	dlg.SetSettings(pref->GetSettings());

	INT_PTR response = dlg.DoModal();

	// パンくずリストを取得(次回表示時にページを復元するため)
	mLastBreakCrumbs = dlg.GetBreadCrumbsString();

	if (response != IDOK) {
		return 0;
	}

	// 設定変更を反映する
	pref->SetSettings(dlg.GetSettings());
	pref->Save();

	// 
	auto mainWnd = launcherapp::mainwindow::controller::MainWindowController::GetInstance();
	// 状態クリア
	mainWnd->ClearContent();
	// ウインドウ非表示
	mainWnd->HideWindow();
	// ウインドウ再表示
	bool isShowToggle = true;
	mainWnd->ActivateWindow(isShowToggle);

	return 0;
}

HICON SettingCommand::GetIcon()
{
	return IconLoader::Get()->LoadSettingIcon();

}


launcherapp::core::Command* SettingCommand::Clone()
{
	return new SettingCommand(*this);
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace launcherapp

