#include "pch.h"
#include "ShellExecSettingDialog.h"
#include "commands/shellexecute/CommandEditDialog.h"
#include "commands/shellexecute/ShellExecEditEnvPage.h"
#include "commands/shellexecute/ShellExecEditDetailPage.h"
#include "utility/ScopeAttachThreadInput.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace launcherapp {
namespace commands {
namespace shellexecute {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct SettingDialog::PImpl
{
	// 設定情報
	 CommandParam mParam;
	 CString mOrgName;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



SettingDialog::SettingDialog(CWnd* parentWnd) :
	SettingDialogBase(parentWnd),
 	in(std::make_unique<PImpl>())
{
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::SetName(const CString& name)
{
	in->mParam.mName = name;
}

void SettingDialog::SetOriginalName(const CString& name)
{
	in->mOrgName = name;
}

void SettingDialog::SetParam(const CommandParam& param)
{
	in->mParam = param;
}

const CommandParam&
SettingDialog::GetParam()
{
	return in->mParam;
}

void SettingDialog::ResetHotKey()
{
	in->mParam.mHotKeyAttr.Reset();
}

void SettingDialog::DoDataExchange(CDataExchange* pDX)
{
	__super::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(SettingDialog, SettingDialogBase)
END_MESSAGE_MAP()


BOOL SettingDialog::OnInitDialog()
{
	__super::OnInitDialog();


	CString caption(_T("コマンドの設定"));

	CString suffix;
	suffix.Format(_T("【%s】"), in->mOrgName.IsEmpty() ? _T("新規作成") : (LPCTSTR)in->mOrgName);

	caption += suffix;
	SetWindowText(caption);

	ScopeAttachThreadInput scope;
	SetForegroundWindow();

	return TRUE;
}

HTREEITEM SettingDialog::OnSetupPages()
{

	void* param = &(in->mParam);

	// 各ページ作成
	auto dlg1 = new CommandEditDialog(this);
	dlg1->SetOriginalName(in->mOrgName);
	HTREEITEM hItem = AddPage(TVI_ROOT, std::move(std::unique_ptr<SettingPage>(dlg1)), param);
	AddPage(TVI_ROOT, std::move(std::unique_ptr<SettingPage>(new SettingPageEnv(this))), param);
	AddPage(TVI_ROOT, std::move(std::unique_ptr<SettingPage>(new ShellExecEditDetailPage(this))), param);

	return hItem;
}


}
}
}
