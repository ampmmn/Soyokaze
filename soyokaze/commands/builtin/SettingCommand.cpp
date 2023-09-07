#include "pch.h"
#include "framework.h"
#include "SettingCommand.h"
#include "gui/SettingDialog.h"
#include "AppPreference.h"
#include "IconLoader.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
namespace commands {
namespace builtin {

CString SettingCommand::TYPE(_T("Builtin-Setting"));

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

SettingCommand::~SettingCommand()
{
}

BOOL SettingCommand::Execute(const Parameter& param)
{
	if (mIsExecuting) {
		// 既に実行中
		return TRUE;
	}

	struct scope_flag {
		scope_flag(bool& f) : mf(f) { mf= true; }
		~scope_flag() { mf = false; }
		bool& mf;
	} _local(mIsExecuting);

	SettingDialog dlg;

	// 前回表示していたページを復元する
	dlg.SetBreadCrumbsString(mLastBreakCrumbs);

	auto pref = AppPreference::Get();

	// 現在の設定をセット
	dlg.SetSettings(pref->GetSettings());

	INT_PTR response = dlg.DoModal();

	// パンくずリストを取得(次回表示時にページを復元するため)
	mLastBreakCrumbs = dlg.GetBreadCrumbsString();

	if (response != IDOK) {
		return TRUE;
	}

	// 設定変更を反映する
	pref->SetSettings(dlg.GetSettings());
	pref->Save();


	return TRUE;
}

HICON SettingCommand::GetIcon()
{
	return IconLoader::Get()->LoadSettingIcon();

}


soyokaze::core::Command* SettingCommand::Clone()
{
	return new SettingCommand();
}

} // end of namespace builtin
} // end of namespace commands
} // end of namespace soyokaze

