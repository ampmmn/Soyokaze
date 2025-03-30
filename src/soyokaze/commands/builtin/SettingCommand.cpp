#include "pch.h"
#include "framework.h"
#include "SettingCommand.h"
#include "settingwindow/AppSettingDialog.h"
#include "setting/AppPreference.h"
#include "icon/IconLoader.h"
#include "SharedHwnd.h"
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


	SharedHwnd sharedHwnd;
	::SendMessage(sharedHwnd.GetHwnd(), WM_APP+17, (WPARAM)CallbackExecute, (LPARAM)this);
	return TRUE;
}

LRESULT SettingCommand::CallbackExecute(LPARAM lparam)
{
	SettingCommand* pThis = (SettingCommand*)lparam;
	return pThis->OnCallbackExecute();
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
	SharedHwnd sharedHwnd;
	// 状態クリア
	::SendMessage(sharedHwnd.GetHwnd(), WM_APP+18, 0, 0);
	// ウインドウ非表示
	::SendMessage(sharedHwnd.GetHwnd(), WM_APP+7, 0, 0);
	// ウインドウ再表示
	::SendMessage(sharedHwnd.GetHwnd(), WM_APP+2, 0, 0);

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

