#include "pch.h"
#include "WindowActivateSettingDialog.h"
#include "commands/activate_window/WindowActivateCommandParam.h"
#include "gui/CommandHotKeyDialog.h"
#include "utility/ScopeAttachThreadInput.h"
#include "utility/TopMostMask.h"
#include "resource.h"

namespace soyokaze {
namespace commands {
namespace activate_window {

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



struct SettingDialog::PImpl
{
	// 設定情報
	CommandParam mParam;
	CString mOrgName;

	// メッセージ欄
	CString mMessage;

	// ホットキー(表示用)
	CString mHotKey;

	TopMostMask mTopMostMask;
};

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////



SettingDialog::SettingDialog() : 
	CDialogEx(IDD_WINDOWACTIVATEEDIT),
	in(std::make_unique<PImpl>())
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
	DDX_Text(pDX, IDC_STATIC_STATUSMSG, in->mMessage);
	DDX_Text(pDX, IDC_EDIT_NAME, in->mParam.mName);
	DDX_Text(pDX, IDC_EDIT_DESCRIPTION, in->mParam.mDescription);
	DDX_Text(pDX, IDC_EDIT_CAPTION, in->mParam.mCaptionStr);
	DDX_Text(pDX, IDC_EDIT_CLASS, in->mParam.mClassStr);
	DDX_Check(pDX, IDC_CHECK_REGEXP, in->mParam.mIsUseRegExp);
	DDX_Text(pDX, IDC_EDIT_HOTKEY, in->mHotKey);
}

BEGIN_MESSAGE_MAP(SettingDialog, CDialogEx)
	ON_COMMAND(IDC_BUTTON_HOTKEY, OnButtonHotKey)
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

	in->mHotKey = in->mParam.mHotKeyAttr.ToString();

	ScopeAttachThreadInput scope;
	SetForegroundWindow();

	UpdateData(FALSE);

	return TRUE;
}

void SettingDialog::OnButtonHotKey()	
{
	UpdateData();

	CommandHotKeyDialog dlg(in->mParam.mHotKeyAttr);
	dlg.mIsGlobal = in->mParam.mIsGlobal;
	if (dlg.DoModal() != IDOK) {
		return ;
	}

	dlg.GetAttribute(in->mParam.mHotKeyAttr);
	in->mParam.mIsGlobal = dlg.IsGlobal();
	in->mHotKey = in->mParam.mHotKeyAttr.ToString();

	UpdateData(FALSE);
}

void SettingDialog::OnOK()
{
	UpdateData();

	if (in->mParam.mCaptionStr.IsEmpty() &&
			in->mParam.mClassStr.IsEmpty()) {
		AfxMessageBox(_T("ウインドウタイトルかウインドウクラスを入力してください"));
		return;
	}

	CString msg;
	if (in->mParam.BuildCaptionRegExp(&msg)  == false) {
		AfxMessageBox(msg);
		GetDlgItem(IDC_EDIT_CAPTION)->SetFocus();
		return;
	}

	if (in->mParam.BuildClassRegExp(&msg)  == false) {
		AfxMessageBox(msg);
		GetDlgItem(IDC_EDIT_CLASS)->SetFocus();
		return;
	}

	__super::OnOK();
}


}
}
}
