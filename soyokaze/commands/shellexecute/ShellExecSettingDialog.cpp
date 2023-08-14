#include "pch.h"
#include "ShellExecSettingDialog.h"
#include "commands/shellexecute/CommandEditDialog.h"
#include "commands/shellexecute/ShellExecEditDetailPage.h"
#include "utility/ScopeAttachThreadInput.h"
#include "resource.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

namespace soyokaze {
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



SettingDialog::SettingDialog() : in(std::make_unique<PImpl>())
{
}

SettingDialog::~SettingDialog()
{
}

void SettingDialog::SetParam(const Param& param)
{
	in->mParam = param;
}

const SettingDialog::Param&
SettingDialog::GetParam()
{
	return in->mParam;
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

	in->mOrgName = in->mParam.mName;

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
	HTREEITEM hItem = AddPage(TVI_ROOT, std::unique_ptr<SettingPage>(new CommandEditDialog(this)), param);
	AddPage(TVI_ROOT, std::unique_ptr<SettingPage>(new ShellExecEditDetailPage(this)), param);

	return hItem;
}


}
}
}
